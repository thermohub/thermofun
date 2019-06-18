#include "AbstractData.h"
#include "ElementData.h"

// C++ includes
#include <functional>
#include "jsonio/jsondomfree.h"
// ThermoFun includes
#ifdef FROM_SRC
#include "../thermofun/OptimizationUtils.h"
#else
#include "thermofun/OptimizationUtils.h"
#endif
#include "sourcetdb.h"

using namespace std;
using namespace jsonio;

namespace ThermoFun {

using QueryRecord           = std::function<string(string, vector<string>)>;
using LoadRecord            = std::function<jsonio::FieldSetMap(const string, const vector<string>)>;
using LoadRecords           = std::function<jsonio::ValuesTable(const vector<string>)>;

using QueryInEdgesDefines   = std::function<vector<string>(string, string)>;
using DefinesReactionSymbol = std::function<string(string, string)>;

using QueryInEdgesTakes     = std::function<vector<string>(string)>;
using ReactantsCoeff        = std::function<std::map<string, double>(string)>;

using GetJsonRecord           = std::function<std::string(std::string)>;
//using GetJsonBsonRecord       = std::function<std::pair<std::string, bson>(std::string)>;

// static std::shared_ptr<bsonio::TDBGraph> dbc = std::shared_ptr<bsonio::TDBGraph>();
//const string ThermoDataSetQueryEdges = " basis, pulls, involves, master, prodreac, takes, defines, product ";
const string ThermoDataSetQueryEdges = " basis, pulls, involves, master, prodreac, takes, defines, product ";

struct AbstractData::Impl
{
    /// Vertex name
    const string name;
    /// Default query to vertex
    const DBQueryData query;
    /// Paths to fields to be extracted
    vector<string> fieldPaths;
    /// Columns headers names ( for extracted data )
    vector<string> dataHeaders;
    /// Data names which will be used throughout the code
    vector<string> dataNames;

    std::map<std::string, uint> dataIndex;
    std::map<std::string, std::string> dataPath;

    std::map<std::string, std::string> substSymbolLevel;

    /// Data base connection
    const jsonio::TDataBase* dbconnect;
    std::shared_ptr<jsonio::TDBVertexDocument> dbvertex;

    /// Data base connection access to all data types
    std::shared_ptr<jsonio::TDBEdgeDocument> dbedge_all;

    QueryRecord  query_record_fn;
    LoadRecord   load_record_fn;
    LoadRecords  load_records_fn;

    QueryInEdgesDefines   query_in_edges_defines_fn;
    DefinesReactionSymbol defines_reaction_symbol_fn;

    QueryInEdgesTakes   query_in_edges_takes_fn;
    ReactantsCoeff      reactans_coeff_fn;

    // function for retrieving the full JSON record from the database using the record id
    GetJsonRecord get_json_record_fn_;
    GetJsonRecord get_json_bson_record_v_fn;
    GetJsonRecord get_json_bson_record_e_fn;

    /// Reconect to other data base
    void  changeDataBase( const jsonio::TDataBase* adbconnect )
    {
      dbconnect = adbconnect;
//      auto v = jsonio::TDBVertexDocument::newDBVertexDocument( dbconnect, name, query );
      dbvertex.reset( jsonio::TDBVertexDocument::newVertexDocument( dbconnect, name ) );
      dbedge_all.reset( jsonio::documentAllEdges( dbconnect ) );
      // clear old data
      dataIndex.clear();
      dataPath.clear();
      substSymbolLevel.clear();
    }

    /// Internl functiond for using internal selection part API
    void loadQuery()
    {
      dbvertex->SetQuery( query, fieldPaths );
    }

    Impl(   const jsonio::TDataBase* adbconnect,
            const string &aname, const DBQueryData& aquery, const vector<string> &apaths,
            const vector<string> &headers, const vector<string> &names) :
            name(aname), query(aquery), fieldPaths(apaths),
            dataHeaders(headers), dataNames(names), dbconnect(adbconnect)
    {
        // init connections to data base
        changeDataBase( adbconnect );

        get_json_bson_record_v_fn = [=](string idRecord) {
            return getJsonRecordVertex(idRecord);
        };
        get_json_bson_record_v_fn = memoize(get_json_bson_record_v_fn);

        get_json_bson_record_e_fn = [=](string idRecord) {
            return getJsonRecordEdge(idRecord);
        };
        get_json_bson_record_e_fn = memoize(get_json_bson_record_e_fn);

        load_record_fn = [=](const string idRecord, const vector<string> queryFields) {
            return loadRecord(idRecord, queryFields);
        };
        load_record_fn = memoize(load_record_fn);

        load_records_fn = [=](const vector<string> idRecords) {
            return loadRecords(idRecords);
        };
        load_records_fn = memoize(load_records_fn);

        query_in_edges_defines_fn = [=](string idSubst,  string level) {
            return queryInEdgesDefines(idSubst, level);
        };
        query_in_edges_defines_fn = memoize(query_in_edges_defines_fn);

        defines_reaction_symbol_fn = [=](string idSubst, string level) {
            return definesReactionSymbol(idSubst, level);
        };
        defines_reaction_symbol_fn = memoize(defines_reaction_symbol_fn);

        query_in_edges_takes_fn = [=](string idReact) {
            return queryInEdgesTakes(idReact);
        };
        query_in_edges_takes_fn = memoize(query_in_edges_takes_fn);

        reactans_coeff_fn = [=](string idReact) {
            return reactantsCoeff(idReact);
        };
        reactans_coeff_fn = memoize(reactans_coeff_fn);
    }

    auto getJsonRecordVertex(string idRecord) -> std::string // id: "12234444" format
    {
        std::string json_str;
        dbvertex->resetMode(true);
        dbvertex->Read(idRecord);
        json_str = dbvertex->GetJson();
        dbvertex->resetMode(false);
        return json_str;
    }

    auto getJsonRecordEdge(string idRecord) -> std::string // id: "12234444" format
    {
        std::string json_str;
        dbedge_all->Read(idRecord);
        json_str = dbedge_all->GetJson();
        return json_str;
    }

    auto loadRecord(string idRecord, vector<string> queryFields) -> jsonio::FieldSetMap
    {
        return dbvertex->loadRecordFields(idRecord, queryFields);
    }

    auto loadRecords( vector<string> idRecords) -> jsonio::ValuesTable
    {
        dbvertex->resetSchema(name, false);
        return dbvertex->downloadDocuments(idRecords, fieldPaths);
    }

    // return all record, no only fields
    auto queryInEdgesDefines(string idSubst,  string level) -> vector<string>
    {
        string qrJson = "FOR e IN defines";
               qrJson += "\n FILTER e._to == '" + idSubst + "'";
               if (level != "-1")
                  qrJson += " && e.properties.level ==" + level;
               qrJson += "\n RETURN e._from";

        vector<string> resultsEdgeIds =  dbedge_all->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ) );

        for( uint ii=0; ii<resultsEdgeIds.size(); ii++  )
            jsonio::strip_all( resultsEdgeIds[ii] , "\"");

        return resultsEdgeIds;
    }

    auto definesReactionSymbol(string idSubst, string level) -> std::string
    {
        string qrJson = "FOR v,e  IN 1..1 INBOUND '";
               qrJson += idSubst + "' \n defines\n";
               if (level != "-1")
                  qrJson += "FILTER e.properties.level ==" + level +"\n";
               qrJson += "RETURN v.properties.symbol";

        vector<string> symbols = dbedge_all->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ) );
        if( symbols.empty() )
          return ""; //"undefined";
        jsonio::strip_all( symbols[0] , "\"");
        return symbols[0];
    }

    // return all record, no only fields
    auto queryInEdgesTakes(string idReact) -> vector<string>
    {
        auto queryin = dbedge_all->inEdgesQuery( "takes", idReact );
        vector<string> resultEdge =  dbedge_all->runQuery( queryin );
        return resultEdge;
        //"FOR v,e  IN 1..1 INBOUND 'reactions/OH-_1_15_1' takes RETURN e"
    }

    auto reactantsCoeff(string idReact) -> std::map<std::string, double>
    {
        std::map<std::string, double> reactantsCoeff;
        string coeff, symbol;
        double stoi_coeff;

        string qrJson = "FOR v,e  IN 1..1 INBOUND '";
               qrJson += idReact + "' \n takes\n";
               qrJson += "RETURN { 'stoi_coeff': e.properties.stoi_coeff, 'symbol': v.properties.symbol }";

        vector<string> resultsQuery =  dbedge_all->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ) );

        for( auto result: resultsQuery )
        {
            auto domdata = jsonio::unpackJson( result );
            domdata->findValue("stoi_coeff", coeff);
            stoi_coeff = atof(coeff.c_str());
            domdata->findValue("symbol", symbol);
            reactantsCoeff.insert(std::pair<std::string,double>(symbol, stoi_coeff));
        }
        return reactantsCoeff;
    }
};


AbstractData::AbstractData( const jsonio::TDataBase* dbconnect, const string &name, const DBQueryData& query,
                            const vector<string> &paths, const vector<string> &headers, const vector<string> &names)
    : pimpl(new Impl( dbconnect, name, query, paths, headers, names))
{
    resetDataPathIndex();
}

AbstractData::AbstractData(const AbstractData& other)
    : pimpl(new Impl(*other.pimpl))
{}

AbstractData::~AbstractData()
{ }

auto AbstractData::getJsonRecordVertex(string idRecord) -> std::string
{
    return pimpl->get_json_bson_record_v_fn(idRecord);
}

auto AbstractData::getJsonRecordEdge(string idRecord) -> std::string
{
    return pimpl->get_json_bson_record_e_fn(idRecord);
}

auto AbstractData::queryRecord(string idRecord, vector<string> queryFields) -> string
{
    return pimpl->query_record_fn(idRecord, queryFields);
}
auto AbstractData::queryInEdgesDefines_(string idSubst,  string level) -> vector<string>
{
    return pimpl->query_in_edges_defines_fn(idSubst, level);
}

auto AbstractData::definesReactionSymbol_(string idSubst, string level) -> std::string
{
    return pimpl->defines_reaction_symbol_fn(idSubst, level);
}

auto AbstractData::queryInEdgesTakes_(string idReact) -> vector<string>
{
    return pimpl->query_in_edges_takes_fn(idReact);
}

auto AbstractData::reactantsCoeff_(string idReact) -> std::map<std::string, double>
{
    return pimpl->reactans_coeff_fn(idReact);
}

/// Extract values from record into database
auto AbstractData::loadRecord( const string id, const vector<string> queryFields ) -> jsonio::FieldSetMap
{
    return pimpl->load_record_fn( id, queryFields );
}
/// Build table of fields values by ids list
auto AbstractData::loadRecords(const vector<string> ids ) -> jsonio::ValuesTable
{
    //return pimpl->loadRecords(ids);
    return pimpl->load_records_fn(ids);
}

auto AbstractData::getName() const -> string
{
    return pimpl->name;
}

auto AbstractData::getQuery() const -> DBQueryData
{
    return pimpl->query;
}

// Execute query before load impex to compare data
auto  AbstractData::loadQueryData() -> void
{
   pimpl->loadQuery();
}

auto AbstractData::getDataFieldPaths() const -> vector<string>
{
    return pimpl->fieldPaths;
}


auto AbstractData::setDataFieldPaths(const vector<string> &value) -> void
{
    pimpl->fieldPaths = value;
}

auto AbstractData::getDataHeaders() const -> vector<string>
{
    return pimpl->dataHeaders;
}

auto AbstractData::setDataHeaders(const vector<string> &value) -> void
{
    pimpl->dataHeaders = value;
}

auto AbstractData::getDataNames() const -> vector<string>
{
    return pimpl->dataNames;
}

/*auto AbstractData::setDataNames(const vector<string> &value) -> void
{
    pimpl->dataNames = value;
}*/

auto AbstractData::getDB() const -> std::shared_ptr<jsonio::TDBVertexDocument>
{
    return pimpl->dbvertex;
}


/*
auto AbstractData::setDB(const std::shared_ptr<bsonio::TDBGraph> &value) -> void
{
    pimpl->graphdb     = value;
    pimpl->graphdb_all = value;
    pimpl->graphdb_all->resetMode(true);
}*/

auto AbstractData::setDataNamesHeadersFieldpaths(const vector<string> &names, const vector<string> &headers, const vector<string> &fieldpaths) -> void
{
    pimpl->dataNames = names;
    pimpl->dataHeaders = headers;
    pimpl->fieldPaths = fieldpaths;
    resetDataPathIndex();
}

auto AbstractData::makeQueryFields() const -> jsonio::QueryFields
{
   jsonio::QueryFields fldsMap;
   for(uint ii=0; ii<pimpl->fieldPaths.size() ; ii++ )
   {
      auto fieldpath = replace(pimpl->fieldPaths[ii], "values.0", "values[0]");
      fldsMap[pimpl->dataNames[ii]] = fieldpath;
   }
   return fldsMap;
}

bool AbstractData::recordExists(const string& id )
{
    return getDB()->Find( id );
}

auto AbstractData::CreateRecord( const jsonio::FieldSetMap& fldvalues, bool testValues ) -> string
{
    return getDB()->CreateVertex( getName(), fldvalues, testValues );
}

auto AbstractData::resetDataPathIndex() -> void
{
    pimpl->dataIndex.clear(); pimpl->dataPath.clear();
    for (uint i = 0; i<getDataNames().size(); i++)
    {
        pimpl->dataIndex[getDataNames()[i]] = i;
        pimpl->dataPath[getDataNames()[i]] = getDataFieldPaths()[i];
    }
}

auto AbstractData::testElementsFormula( const string& aformula,
                                        const vector<ElementKey>& elements ) -> bool
{
    FormulaToken parser(aformula);

    for( auto formelm: parser.getElements() )
    {
        auto itr = elements.begin();
        while( itr != elements.end() )
        {
            if( formelm == *itr )
                break;
            itr++;
        }
        if( itr == elements.end() )
            return false;
    }
    return true;
}

auto AbstractData::getInVertexIds( const string& edgeCollections, const string& idVertex ) -> vector<string>
{
    vector<string> vertexIds_;
    string vertexId_;

    vector<string> _queryFields = { "_from"};
    auto queryin = pimpl->dbedge_all->inEdgesQuery( edgeCollections, idVertex );
    queryin.setQueryFields(_queryFields);

    vector<string> _resultData = pimpl->dbedge_all->runQuery( queryin );

    for( auto rec: _resultData)
    {
        vertexId_ =  jsonio::extractStringField( "_from", rec );
        vertexIds_.push_back(vertexId_);
    }
    return vertexIds_;
}

auto AbstractData::getInVertexIds(const string& edgeCollections, const string& idVertex,  vector<string> &edgeIds_) -> vector<string>
{
    vector<string> vertexIds_;
    string vertexId_, edgeId_;
    edgeIds_.clear();

    vector<string> _queryFields = { "_from", "_id"};
    auto queryin = pimpl->dbedge_all->inEdgesQuery( edgeCollections, idVertex );
    queryin.setQueryFields(_queryFields);

    vector<string> _resultData = pimpl->dbedge_all->runQuery( queryin );

    for( auto rec: _resultData)
    {
        vertexId_ =  jsonio::extractStringField( "_from", rec );
        vertexIds_.push_back(vertexId_);
        edgeId_ =  jsonio::extractStringField( "_id", rec );
        edgeIds_.push_back(edgeId_);
    }
    return vertexIds_;
}

auto AbstractData::getOutVertexIds( const string &edgeCollections, const string& idVertex ) -> vector<string>
{
    vector<string> vertexIds_;
    string vertexId_;

    // select all EdgeTakes for reaction
    auto queryout = pimpl->dbedge_all->outEdgesQuery( edgeCollections, idVertex );
    vector<string> _queryFields = { "_to"};
    queryout.setQueryFields(_queryFields);

    vector<string> _resultData = pimpl->dbedge_all->runQuery( queryout );

    for( auto rec: _resultData)
    {
        vertexId_ =  jsonio::extractStringField( "_to", rec );
        vertexIds_.push_back(vertexId_);
    }
    return vertexIds_;
}

auto AbstractData::getOutVertexIds(const string &edgeCollections, const string& idVertex,  vector<string> &edgeIds_) -> vector<string>
{
    vector<string> vertexIds_;
    string vertexId_, edgeId_;
    edgeIds_.clear();

    vector<string> _queryFields = { "_to", "_id"};
    auto queryout = pimpl->dbedge_all->outEdgesQuery( edgeCollections, idVertex );
    queryout.setQueryFields(_queryFields);

    vector<string> _resultData = pimpl->dbedge_all->runQuery( queryout );

    for( auto rec: _resultData)
    {
        vertexId_ =  jsonio::extractStringField( "_to", rec );
        vertexIds_.push_back(vertexId_);
        edgeId_ =  jsonio::extractStringField( "_id", rec );
        edgeIds_.push_back(edgeId_);
    }
    return vertexIds_;
}

// Connect to new Data Base
auto AbstractData::updateDBClient( const jsonio::TDataBase* newdbconnect ) -> void
{
    try{
        pimpl->changeDataBase(newdbconnect);
    }
    catch(std::exception& e)
    {
        cout << "Internal comment " << e.what() << endl;
        // throw;
    }
}

auto AbstractData::setDefaultLevelForReactionDefinedSubst(jsonio::ValuesTable avaluesTable) -> void
{
    for( const auto& subitem : avaluesTable )
    {
        pimpl->substSymbolLevel[subitem[getDataName_DataIndex()["symbol"]]] = "0";
    }
}

auto AbstractData::getDataName_DataIndex() const -> std::map<std::string, uint>
{
    return pimpl->dataIndex;
}

auto AbstractData::getDataName_DataFieldPath() const -> std::map<std::string, std::string>
{
    return pimpl->dataPath;
}

auto AbstractData::getSubstSymbol_DefinesLevel() const -> std::map<std::string, std::string>
{
    return pimpl->substSymbolLevel;
}

auto AbstractData::setSubstSymbol_DefinesLevel(const std::map<std::string, std::string> &value) -> void
{
    pimpl->substSymbolLevel = value;
}

auto AbstractData::getSubstanceLevel_(string substSymbol) const -> string
{
    return pimpl->substSymbolLevel[substSymbol];
}

auto AbstractData::setSubstanceLevel_(string substSymbol, string level) -> void
{
    pimpl->substSymbolLevel[substSymbol] = level;
}

auto AbstractData::getDB_edgeAccessMode() const -> std::shared_ptr<jsonio::TDBEdgeDocument>
{
    return pimpl->dbedge_all;
}

// delete not unique
void AbstractData::deleteNotUnique(jsonio::ValuesTable& dataMatr, uint fldtestNdx )
{
    ValuesTable newMatr;
    for (const auto& subitem : dataMatr)
    {
       auto symbol = subitem[fldtestNdx];
       if ( newMatr.empty() ||  newMatr.back()[fldtestNdx] != symbol )
                   newMatr.push_back(subitem);
    }
    dataMatr = move(newMatr);
}

// addition functions to work with elemens

auto AbstractData::selectElementsGiven( const vector<int>& sourcetdbs, bool unique ) -> vector<ElementKey>
{
  jsonio::QueryFields qwFields = { {"symbol", "properties.symbol"},
                                   {"class_","properties.class_"},
                                   {"isotope_mass", "properties.isotope_mass"} };
  string AQLreq = "FOR u IN elements \n"
                  "  FILTER u.properties.sourcetdb IN @sourcetdbs\n"
                  "  SORT u.properties.symbol ";
          AQLreq += DBQueryData::generateReturn( unique,  qwFields );

  // generate bind values
  shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
  auto arr = domdata->appendArray( "sourcetdbs");
  sourceTDB_from_indexes( sourcetdbs, arr );
  // make query
  DBQueryData query( AQLreq, DBQueryData::qAQL );
  query.setBindVars( domdata.get() );

  pimpl->dbvertex->resetMode(true);
  vector<string> resultsQuery = pimpl->dbvertex->runQuery( query );
  pimpl->dbvertex->resetMode(false);

  vector<ElementKey> elements;
  ElementKey elkey("",0,0);
  for( auto result: resultsQuery )
  {
      auto domdata = jsonio::unpackJson( result );
      elkey = fromElementNode(domdata.get());
      elements.push_back(elkey);
   }
   return elements;
}

auto AbstractData::selectElementsFromSubstancesGiven( const vector<int>& sourcetdbs ) -> set<ElementKey>
{
    // Select all substances formulas
    string AQLreq = "FOR u IN substances \n"
                    "  FILTER u.properties.sourcetdb IN @sourcetdbs\n"
                    "  SORT u.properties.symbol ";
           AQLreq += "RETURN DISTINCT u.properties.formula";

    // generate bind values
    shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
    auto arr = domdata->appendArray( "sourcetdbs");
    sourceTDB_from_indexes( sourcetdbs, arr );
    // make query
    DBQueryData query( AQLreq, DBQueryData::qAQL );
    query.setBindVars( domdata.get() );

    vector<string> formulas = getDB()->runQuery( query );
    return ChemicalFormula::extractElements(formulas);
}


}
