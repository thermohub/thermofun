#include "DatabaseClient.h"

// C++ includes
//#include <functional>

// bonio includes
#include "jsonio/traversal.h"
#include "jsonio/io_settings.h"

// ThermoFun includes
#include "SubstanceData.h"
#include "ReactionData.h"
#include "ReactionSetData.h"
#include "ThermoSetData.h"
#include "TraversalData.h"

#include "Database.h"
#include "Element.h"
#include "OptimizationUtils.h"

using namespace jsonio;

namespace ThermoFun
{

using QuerySubstancesFunction = std::function<std::vector<std::string>(uint)>;
using QueryReactionsFunction  = std::function<std::vector<std::string>(uint)>;
using AvailableElementsSet    = std::function<set<Element>(uint)>;
using AvailableElementsKey    = std::function<std::vector<ElementKey>(uint)>;

//std::vector<std::string> queryFieldsSubstance    = {"_id", "properties.formula", "properties.symbol", "properties.sourcetdb"};
//std::vector<std::string> queryFieldsReaction     = {"_id", "properties.equation", "properties.symbol", "properties.sourcetdb"};

struct DatabaseClient::Impl
{
    std::shared_ptr<TDataBase> _dbconnect;

    /// access to substance records
    SubstanceData_ substData;

    /// access to reaction records
    ReactionData_ reactData;

    /// access to reactionSet records
    ReactionSetData_ reactSetData;

    /// acccess to ThermoDataSet records
    ThermoSetData thermoDataSet;

    /// for traversal operations
    TraversalData traversal;

    QuerySubstancesFunction query_substances_fn;

    QueryReactionsFunction query_reactions_fn;

    AvailableElementsSet available_elements_set_fn;

    AvailableElementsKey available_elements_key_fn;

    Impl(const std::shared_ptr<TDataBase>& otherdb) :
      _dbconnect( otherdb), substData(_dbconnect.get()),
      reactData(_dbconnect.get()), reactSetData(_dbconnect.get()),
      thermoDataSet(_dbconnect.get()), traversal (&substData, &reactData)
    {
        setFunctions();
    }

    // read from default config
    Impl() : _dbconnect( new TDataBase() ), substData(_dbconnect.get()),
        reactData(_dbconnect.get()), reactSetData(_dbconnect.get()),
        thermoDataSet(_dbconnect.get()), traversal (&substData, &reactData)
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

        available_elements_key_fn = [=](uint sourcetdb) {
            return availableElementsKey(sourcetdb);
        };
        available_elements_key_fn = memoize(available_elements_key_fn);

        available_elements_set_fn = [=](uint sourcetdb) {
            return availableElementsSet(sourcetdb);
        };
        available_elements_set_fn = memoize(available_elements_set_fn);

        auto elementVertex = unique_ptr<TDBVertexDocument> (
                    TDBVertexDocument::newVertexDocument( _dbconnect.get(),  "VertexElement" ));
        // load all elements into system
        ChemicalFormula::setDBElements( elementVertex.get(), ChemicalFormula::getDefaultQuery() );
    }

    auto availableElementsSet(int sourcetdb) -> set<Element>
    {
        std::set<ElementKey> els_;
        std::set<Element> set;

        auto _resultData = query_substances_fn(sourcetdb);


        FormulaToken parser("");
        for (string subitem: _resultData)
        {
            string formula = extractStringField("formula", subitem);
            //string symbol = bsonio::extractStringField("symbol", subitem);
            // cout <<  formula << "  " << symbol << endl;
            // test elements
            // addiditon test and parser parser.exeptionCheckElements(symbol, formula);
    //        parser.exeptionCheckElements(symbol, formula);
            parser.setFormula(formula);
            els_.insert(parser.getElements().begin(), parser.getElements().end());
        }

        for (auto el : els_)
        {
            auto itrdb = ChemicalFormula::getDBElements().find(el);
            if (itrdb == ChemicalFormula::getDBElements().end())
                jsonioErr("E37FPrun: Invalid symbol ", el.symbol);
            Element e = elementKeyToElement(el);
            set.insert(e);
        }
        return set;
    }

    auto availableElementsKey(uint sourcetdb) -> std::vector<ElementKey>
    {
        std::set<ElementKey> elements;
        std::vector<ElementKey> set;

        auto _resultData = query_substances_fn(sourcetdb);

        FormulaToken parser("");
        for (string subitem: _resultData)
        {
            string formula = extractStringField("formula", subitem);
            string symbol = extractStringField("symbol", subitem);
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
                jsonioErr("E37FPrun: Invalid symbol ", element.symbol);
            set.push_back(element);
        }
        return set;
    }

    auto querySubstances(uint sourcetdb) -> std::vector<std::string>
    {
        string query = "{ \"_label\" : \"substance\", \"_type\" : \"vertex\", \"properties.sourcetdb\" : ";
        query += to_string(sourcetdb);
        query += " }";
        vector<string> _resultData = substData.getDB()->runQuery(DBQueryData( query, DBQueryData::qTemplate ) );
        return _resultData;
    }

    auto queryReactions(uint sourcetdb) -> std::vector<std::string>
    {
        string query = "{ \"_label\" : \"reaction\", \"_type\" : \"vertex\", \"properties.sourcetdb\" : ";
        query += to_string(sourcetdb);
        query += " }";
        vector<string> _resultData = reactData.getDB()->runQuery(DBQueryData( query, DBQueryData::qTemplate ) );
        return _resultData;
    }

    auto executeIncomingTraversal( const vector<string>& ids, GraphElementFunction afunc ) -> void
    {
        GraphTraversal travel( _dbconnect.get() );
        travel.Traversal( true, ids, afunc, GraphTraversal::trIn );
    }
};

DatabaseClient::DatabaseClient(const std::shared_ptr<TDataBase>& otherdb)
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
        values.push_back(extractStringField(fieldName, result));
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
  //std::set<ElementKey> elements;
  std::map<Element, double> map;
  FormulaToken formula("");

  formula.setFormula(formula_);
  //    elements.insert(formula.getElements().begin(), formula.getElements().end());

  for (auto element : formula.getElements_map())
  {
        Element e = elementKeyToElement(element.first);
        map[e] = element.second;
   }

 return map;

}

auto DatabaseClient::sourcetdbIndexes() -> std::set<uint>
{
    //string query = "{ \"$or\" : [ { \"_label\" :   \"substance\" }, { \"_label\" :   \"reaction\"  }"
    //               "], \"_type\" :   \"vertex\"  }";
    // query only by substances (all reactions must connect substance )
    //DBQueryData query("{ \"_label\" :   \"substance\" , \"_type\" :   \"vertex\"  }", DBQueryData::qTemplate );
    set<uint> _sourcetdb;
    //vector<string> _resultData = pimpl->substData.getDB()->fieldQuery("properties.sourcetdb", query);
    vector<string> _resultData = pimpl->substData.getDB()->fieldValues("properties.sourcetdb");
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
    ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
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
    ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
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
    ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
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
    return pimpl->available_elements_set_fn(sourcetdb);
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
    return pimpl->available_elements_key_fn(sourcetdb);
}

auto DatabaseClient::elementIds( const std::vector<ElementKey>& elements) -> std::vector<string>
{
    std::vector<string> elmIds;
    for (auto element : elements)
    {
        auto itrdb = ChemicalFormula::getDBElements().find(element);
        if (itrdb == ChemicalFormula::getDBElements().end())
            jsonioErr("E37FPrun: Invalid symbol ", element.symbol);
        elmIds.push_back(itrdb->second.recid);
    }
    return elmIds;
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

auto DatabaseClient::thermoDataSet() const -> ThermoSetData&
{
    return pimpl->thermoDataSet;
}

auto DatabaseClient::getTraversal() const -> TraversalData&
{
    return pimpl->traversal;
}

/// Output record and all incoming to json file
auto DatabaseClient::BackupAllIncoming( const vector<string>& ids, const string fileName ) -> void
{
    FJsonArray file( fileName);
    file.Open( OpenModeTypes::WriteOnly );

    GraphElementFunction afunc =  [&file]( bool , const string& data )
            {
               file.SaveNext(data);
            };

    pimpl->executeIncomingTraversal( ids, afunc );

    file.Close();

}

// Collect record and all incoming _ids
auto DatabaseClient::TraverseAllIncomingEdges( const string& id ) -> List_VertexId_VertexType
{
    List_VertexId_VertexType list;

    GraphElementFunction afunc =  [&list]( bool vert, const string& jsondata )
            {
              if( vert)
              {
                  string label = extractStringField( "_label", jsondata );
                  string id = extractStringField( "_id", jsondata );

                  //string type, id;
                  //data->findKey( "_id",    id );
                  //data->findKey( "_label",  type);
                  list.push_back(pair<string,string>(id, label));
              }
            };
    pimpl->executeIncomingTraversal( {id}, afunc );
    return list;
}

auto DatabaseClient::recordsFromThermoDataSet( const string& ThermoDataSetSymbol ) -> List_VertexId_VertexType
{
    auto idThermoDataSet = thermoDataSet().idRecordFromSymbol(ThermoDataSetSymbol);
    return TraverseAllIncomingEdges(idThermoDataSet);
}

}
