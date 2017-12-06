#include "DatabaseClient.h"

// C++ includes
#include <functional>

// bonio includes
//#include "bsonio/thrift_schema.h"
//#include "bsonio/dbgraph.h"
//#include "bsonio/dbdriverejdb.h"
//#include "bsonio/dbclient.h"
#include "bsonio/io_settings.h"

// ThermoFun includes
#include "SubstanceData.h"
#include "ReactionData.h"
#include "ReactionSetData.h"
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
    std::shared_ptr<bsonio::TDataBase> _dbconnect;

    /// access to substance records
    SubstanceData_ substData;

    /// access to reaction records
    ReactionData_ reactData;

    /// access to reactionSet records
    ReactionSetData_ reactSetData;

    /// for traversal operations
    TraversalData traversal;

    QuerySubstancesFunction query_substances_fn;

    QueryReactionsFunction query_reactions_fn;

    Impl(const std::shared_ptr<bsonio::TDataBase>& otherdb) :
      _dbconnect( otherdb), substData(_dbconnect.get()),
      reactData(_dbconnect.get()), reactSetData(_dbconnect.get()),
      traversal (&substData, &reactData)
    {
        setFunctions();
    }

    // read from default config
    Impl() : _dbconnect( new bsonio::TDataBase() ), substData(_dbconnect.get()),
        reactData(_dbconnect.get()), reactSetData(_dbconnect.get()),
        traversal (&substData, &reactData)
    {
        setFunctions();
    }

    auto setFunctions() -> void
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

        qrJson = "{ \"_label\" : \"element\"}";
        auto elementVertex = unique_ptr<bsonio::TDBVertexDocument> (
                    TDBVertexDocument::newDBVertexDocument(
              _dbconnect.get(),  "VertexElement", qrJson ));
        // load all elements into system
        ChemicalFormula::setDBElements( elementVertex.get(), qrJson );
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

DatabaseClient::DatabaseClient(const std::shared_ptr<bsonio::TDataBase>& otherdb)
    : pimpl(new Impl( otherdb ))
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
    return extractFieldValuesFromQueryResult(pimpl->query_substances_fn(sourcetdb), "symbol");
}

auto DatabaseClient::availableReactions(uint sourcetdb) -> std::vector<std::string>
{
    return extractFieldValuesFromQueryResult(pimpl->query_reactions_fn(sourcetdb), "symbol");
}

auto DatabaseClient::extractFieldValuesFromQueryResult(std::vector<std::string> resultQuery, std::string fieldName) -> std::vector<std::string>
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
    // get substances ids
    auto substKeyList = extractFieldValuesFromQueryResult(pimpl->query_substances_fn(sourcetdbIndex), "_id");

//    for (auto &key_: substKeyList)
//        key_ += ":";
    // get all ids conected to the keyList (by incoming edges, e.g. defines reactions and thier reactants)
    auto resultTraversal = pimpl->traversal.getMapOfConnectedIds(substKeyList, "0");

    return pimpl->traversal.getDatabaseFromMapOfIds(resultTraversal, "0");
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
        for (int idx: sourcetdbIndexes)
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
        for (int idx: sourcetdbIndexes)
        {
            string name = enumdef->getNamebyId(idx);
            namesComments[name] = enumdef->getDoc(name);
        }
    }
    return namesComments;
}

auto DatabaseClient::sourcetdbListAll() -> std::vector<string>
{
    string name, comment;
    vector<string> _sourcetdbList;
    auto indexes = sourcetdbIndexes();
    bsonio::ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
    if (enumdef != nullptr)
    {
        for (int idx: indexes)
        {
            name = enumdef->getNamebyId(idx);
            comment = enumdef->getDoc(name);
            _sourcetdbList.push_back(std::to_string(idx)+"-"+ name + " - "+ comment );
        }
    }
    return _sourcetdbList;
}

auto DatabaseClient::availableElementsSet(int sourcetdb) -> set<Element>
{
    std::set<ElementKey> elements;
    std::set<Element> set;

    auto _resultData = pimpl->query_substances_fn(sourcetdb);

    FormulaToken parser("");
    for (string subitem: _resultData)
    {
        string formula = bsonio::extractStringField("formula", subitem);
        //string symbol = bsonio::extractStringField("symbol", subitem);
        //  cout << subitem << "      " << formula << "  " << symbol << endl;
        // test elements
        // addiditon test and parser parser.exeptionCheckElements(symbol, formula);
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
    for (string subitem: _resultData)
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

auto DatabaseClient::substData() const -> SubstanceData_&
{
    return pimpl->substData;
}

auto DatabaseClient::reactData() const -> ReactionData_&
{
    return pimpl->reactData;
}

auto DatabaseClient::reactSetData() const -> ReactionSetData_&
{
    return pimpl->reactSetData;
}

auto DatabaseClient::getTraversal() const -> TraversalData&
{
    return pimpl->traversal;
}


}
