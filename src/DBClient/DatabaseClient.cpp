#include "DatabaseClient.h"

// C++ includes
#include <functional>

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"
#include "bsonio/dbdriverejdb.h"
#include "bsonio/dbclient.h"
#include "bsonio/io_settings.h"

// ThermoFun includes
#include "SubstanceData.h"
#include "ReactionData.h"
#include "TraversalData.h"

#include "../Database.h"
#include "../Element.h"
#include "../OptimizationUtils.h"

using namespace bsonio;

namespace ThermoFun
{

using QuerySubstancesFunction = std::function<std::vector<std::string>(uint)>;
using QueryReactionsFunction  = std::function<std::vector<std::string>(uint)>;


std::vector<std::string> queryFieldsSubstance    = {"_id", "properties.formula", "properties.symbol", "properties.sourcetdb"};
std::vector<std::string> queryFieldsReaction     = {"_id", "properties.equation", "properties.symbol", "properties.sourcetdb"};

struct DatabaseClient::Impl
{
    /// access to substance records
    SubstanceData substData;

    /// access to reaction records
    ReactionData reactData;

    /// for traversal operations
    TraversalData traversal;

    QuerySubstancesFunction query_substances_fn;

    QueryReactionsFunction query_reactions_fn;

    Impl(std::string settingsFile) : traversal (&substData, &reactData)
    {
        bsonio::BsonioSettings::settingsFileName = settingsFile;
        setDBClient();
    }

    Impl() : traversal (&substData, &reactData)
    {
        setDBClient();
    }

    auto setDBClient( ) -> void
    {
        query_substances_fn = [=](uint sourcetdb) {
            return querySubstances(sourcetdb);
        };
        query_substances_fn = memoize(query_substances_fn);

        query_reactions_fn = [=](uint sourcetdb) {
            return queryReactions(sourcetdb);
        };
        query_reactions_fn = memoize(query_reactions_fn);

        string qrJson;

        // default connections to vertexes
        qrJson = "{ \"_label\" : \"substance\" }";
        substData.setDB(boost::shared_ptr<bsonio::TDBGraph>(ioSettings().newDBGraphClient("VertexSubstance", qrJson)));
        qrJson = "{ \"_label\" : \"reaction\" }";
        reactData.setDB(boost::shared_ptr<bsonio::TDBGraph>(ioSettings().newDBGraphClient("VertexReaction", qrJson)));

        qrJson = "{ \"_label\" : \"element\"}";
        auto elementVertex = unique_ptr<bsonio::TDBGraph> (ioSettings().newDBGraphClient( "VertexElement", qrJson ));
        // load all elements into system
        ChemicalFormula::setDBElements( elementVertex.get(), qrJson );
    }

    auto getJsonRecord(string idRecord) -> string // id: "12234444:" format
    {
        auto graphdb_all = substData.getDB();
        graphdb_all->GetRecord( idRecord.c_str() );
        return graphdb_all->GetJson();
    }

    auto querySubstances(uint sourcetdb) -> std::vector<std::string>
    {
        string query = "{ \"_label\" : \"substance\", \"_type\" : \"vertex\", \"properties.sourcetdb\" : ";
        query += to_string(sourcetdb);
        query += " }";
        vector<string> _queryFields = queryFieldsSubstance;
        vector<string> _resultData;
        substData.getDB()->runQuery(query, _queryFields, _resultData);
        return _resultData;
    }

    auto queryReactions(uint sourcetdb) -> std::vector<std::string>
    {
        string query = "{ \"_label\" : \"reaction\", \"_type\" : \"vertex\", \"properties.sourcetdb\" : ";
        query += to_string(sourcetdb);
        query += " }";
        vector<string> _queryFields = queryFieldsReaction;
        vector<string> _resultData;
        reactData.getDB()->runQuery(query, _queryFields, _resultData);
        return _resultData;
    }
};

DatabaseClient::DatabaseClient(std::string settingsFile)
    : pimpl(new Impl(settingsFile))
{
}

DatabaseClient::DatabaseClient()
    : pimpl(new Impl())
{
}

auto DatabaseClient::operator=(DatabaseClient other) -> DatabaseClient &
{
    pimpl = std::move(other.pimpl);
    return *this;
}

DatabaseClient::~DatabaseClient()
{
}

auto elementKeyToElement(ElementKey elementKey) -> Element
{
    Element e;
    auto itrdb = ChemicalFormula::getDBElements().find(elementKey);
    if (itrdb == ChemicalFormula::getDBElements().end())
        bsonio::bsonioErr("E37FPrun: Invalid symbol ", elementKey.symbol);

    e.setClass(elementKey.class_);
    e.setIsotopeMass(elementKey.isotope);
    e.setSymbol(elementKey.symbol);
    e.setName(itrdb->second.name);
    e.setMolarMass(itrdb->second.atomic_mass);
    e.setEntropy(itrdb->second.entropy);
    e.setHeatCapacity(itrdb->second.heat_capacity);
    e.setVolume(itrdb->second.volume);
    e.setValence(itrdb->second.valence);

    return e;
}

auto DatabaseClient::availableSubstances(uint sourcetdb) -> std::vector<std::string>
{
    return recordsFieldValues(pimpl->query_substances_fn(sourcetdb), "symbol");
}

auto DatabaseClient::availableReactions(uint sourcetdb) -> std::vector<std::string>
{
    return recordsFieldValues(pimpl->query_reactions_fn(sourcetdb), "symbol");
}

auto DatabaseClient::recordsFieldValues(std::vector<std::string> resultQuery, std::string fieldName) -> std::vector<std::string>
{
    std::vector<std::string> values;
    for (auto result : resultQuery)
    {
        values.push_back(bsonio::extractStringField(fieldName, result));
    }
    return values;
}

auto DatabaseClient::thermoFunDatabase(uint sourcetdbIndex) -> Database
{
    Database db;
    vector<string> keyList;

    auto substKeyList = recordsFieldValues(pimpl->query_substances_fn(sourcetdbIndex), "_id");

    for (auto &key_: substKeyList)
        key_ += ":";

    // get substances
    keyList.insert(keyList.end(), substKeyList.begin(), substKeyList.end());

    db = pimpl->traversal.getDatabaseFromMapOfIds(pimpl->traversal.getMapOfConnectedIds(keyList, "0"), "0");

    return db;
}

auto DatabaseClient::parseSubstanceFormula(std::string formula_) -> std::map<Element, double>
{
    std::set<ElementKey> elements;
    std::map<Element, double> map;
    FormulaToken formula("");

    formula.setFormula(formula_);
    elements.insert(formula.getElements().begin(), formula.getElements().end());

    for (auto element : elements)
    {
        Element e = elementKeyToElement(element);
        map[e]++;
    }

    return map;
}

auto DatabaseClient::sourcetdbIndexes() -> std::set<uint>
{
    string query = "{ \"$or\" : [ { \"_label\" :   \"substance\" }, { \"_label\" :   \"reaction\"  }"
                   "], \"_type\" :   \"vertex\"  }";
    set<uint> _sourcetdb;
    vector<string> _resultData = pimpl->substData.getDB()->fieldQuery("properties.sourcetdb", query);

    for (uint ii = 0; ii < _resultData.size(); ii++)
    {
        uint asourcetdb = stoi(_resultData[ii]);
        _sourcetdb.insert(asourcetdb);
    }
    return _sourcetdb;
}

auto DatabaseClient::sourcetdbNamesIndexes(const std::set<uint> &sourcetdbIndexes) -> std::map<string, uint>
{
    // set lists
    std::map<string, uint> namesIndexes;
    bsonio::ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
    if (enumdef != nullptr)
    {
        foreach (int idx, sourcetdbIndexes)
        {
            string name = enumdef->getNamebyId(idx);
            namesIndexes[name] = idx;
        }
    }
    return namesIndexes;
}

auto DatabaseClient::sourcetdbNamesComments(const std::set<uint> &sourcetdbIndexes) -> std::map<string, string>
{
    // set lists
    std::map<string, string> namesComments;
    bsonio::ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
    if (enumdef != nullptr)
    {
        foreach (int idx, sourcetdbIndexes)
        {
            string name = enumdef->getNamebyId(idx);
            namesComments[name] = enumdef->getDoc(name);
        }
    }
    return namesComments;
}

auto DatabaseClient::availableElementsSet(int sourcetdb) -> set<Element>
{
    std::set<ElementKey> elements;
    std::set<Element> set;

    auto _resultData = pimpl->query_substances_fn(sourcetdb);

    FormulaToken parser("");
    foreach (string subitem, _resultData)
    {
        string formula = bsonio::extractStringField("formula", subitem);
        string symbol = bsonio::extractStringField("symbol", subitem);
        //  cout << subitem << "      " << formula << "  " << symbol << endl;
        // test elements
        parser.exeptionCheckElements(symbol, formula);
        parser.setFormula(formula);
        elements.insert(parser.getElements().begin(), parser.getElements().end());
    }

    for (auto element : elements)
    {
        Element e = elementKeyToElement(element);
        set.insert(e);
    }
    return set;
}

auto DatabaseClient::availableElements(uint sourcetdb) -> std::set<string>
{
    std::set<string> set;

    auto elements = availableElementsSet(sourcetdb);

    for (auto element : elements)
    {
        set.insert(element.symbol());
    }
    return set;
}

auto DatabaseClient::availableElementsKey(uint sourcetdb) -> std::vector<ElementKey>
{
    std::set<ElementKey> elements;
    std::vector<ElementKey> set;

    auto _resultData = pimpl->query_substances_fn(sourcetdb);

    FormulaToken parser("");
    foreach (string subitem, _resultData)
    {
        string formula = bsonio::extractStringField("formula", subitem);
        string symbol = bsonio::extractStringField("symbol", subitem);
        //  cout << subitem << "      " << formula << "  " << symbol << endl;
        // test elements
        parser.exeptionCheckElements(symbol, formula);
        parser.setFormula(formula);
        elements.insert(parser.getElements().begin(), parser.getElements().end());
    }

    for (auto element : elements)
    {
        auto itrdb = ChemicalFormula::getDBElements().find(element);
        if (itrdb == ChemicalFormula::getDBElements().end())
            bsonio::bsonioErr("E37FPrun: Invalid symbol ", element.symbol);
        set.push_back(element);
    }
    return set;
}

auto DatabaseClient::substData() const -> SubstanceData
{
    return pimpl->substData;
}

auto DatabaseClient::reactData() const -> ReactionData
{
    return pimpl->reactData;
}


}
