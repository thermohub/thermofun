#include "AbstractData.h"

// C++ includes
#include <functional>
#include "jsonio/jsondomfree.h"
// ThermoFun includes
#include "OptimizationUtils.h"

using namespace jsonio;

namespace ThermoFun {

using QueryRecord           = std::function<string(string, vector<string>)>;
using LoadRecord            = std::function<jsonio::FieldSetMap(const string, const vector<string>)>;
using LoadRecords           = std::function<jsonio::ValuesTable(const vector<string>)>;

using QueryInEdgesDefines   = std::function<vector<string>(string, vector<string>, string)>;
using DefinesReactionSymbol = std::function<string(string, string)>;

using QueryInEdgesTakes     = std::function<vector<string>(string, vector<string>)>;
using ReactantsCoeff        = std::function<std::map<string, double>(string)>;

using GetJsonRecord           = std::function<std::string(std::string)>;
//using GetJsonBsonRecord       = std::function<std::pair<std::string, bson>(std::string)>;

// static std::shared_ptr<bsonio::TDBGraph> dbc = std::shared_ptr<bsonio::TDBGraph>();

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

    std::map<std::string, int> dataIndex;
    std::map<std::string, std::string> dataPath;

    std::map<std::string, std::string> substSymbolLevel;

    /// Data base connection
    const jsonio::TDataBase* dbconnect;
    std::shared_ptr<jsonio::TDBVertexDocument> dbvertex;

    /// Data base connection access to all data types
    std::shared_ptr<jsonio::TDBVertexDocument> dbvertex_all;
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
    GetJsonRecord get_json_bson_record_fn_v;
    GetJsonRecord get_json_bson_record_fn_e;

    /// Reconect to other data base
    void  changeDataBase( const jsonio::TDataBase* adbconnect )
    {
      dbconnect = adbconnect;
      dbvertex.reset( jsonio::TDBVertexDocument::newDBVertexDocument( dbconnect, name, query ) );
      dbvertex_all.reset( jsonio::TDBVertexDocument::newDBVertexDocument( dbconnect, name, query ) );
      dbvertex_all->resetMode(true);
      dbedge_all.reset( jsonio::documentAllEdges( dbconnect ) );
      // clear old data
      dataIndex.clear();
      dataPath.clear();
      substSymbolLevel.clear();
    }

    Impl(   const jsonio::TDataBase* adbconnect,
            const string &aname, const DBQueryData& aquery, const vector<string> &apaths,
            const vector<string> &headers, const vector<string> &names) :
            name(aname), query(aquery), fieldPaths(apaths),
            dataHeaders(headers), dataNames(names), dbconnect(adbconnect)
    {
        // init connections to data base
        changeDataBase( adbconnect );

        get_json_record_fn_ = [=](string idRecord) {
            return getJsonRecord(idRecord);
        };
        get_json_record_fn_ = memoize(get_json_record_fn_);

        get_json_bson_record_fn_v = [=](string idRecord) {
            return getJsonRecordVertex(idRecord);
        };
        get_json_bson_record_fn_v = memoize(get_json_bson_record_fn_v);

        get_json_bson_record_fn_e = [=](string idRecord) {
            return getJsonRecordEdge(idRecord);
        };
        get_json_bson_record_fn_e = memoize(get_json_bson_record_fn_e);

        query_record_fn = [=](string idRecord, vector<string> queryFields) {
            return queryRecord(idRecord, queryFields);
        };
        query_record_fn = memoize(query_record_fn);

        load_record_fn = [=](const string idRecord, const vector<string> queryFields) {
            return loadRecord(idRecord, queryFields);
        };
        load_record_fn = memoize(load_record_fn);

        load_records_fn = [=](const vector<string> idRecords) {
            return loadRecords(idRecords);
        };
        load_records_fn = memoize(load_records_fn);

        query_in_edges_defines_fn = [=](string idSubst, vector<string> queryFields,  string level) {
            return queryInEdgesDefines(idSubst, queryFields, level);
        };
        query_in_edges_defines_fn = memoize(query_in_edges_defines_fn);

        defines_reaction_symbol_fn = [=](string idSubst, string level) {
            return definesReactionSymbol(idSubst, level);
        };
        defines_reaction_symbol_fn = memoize(defines_reaction_symbol_fn);

        query_in_edges_takes_fn = [=](string idReact, vector<string> queryFields) {
            return queryInEdgesTakes(idReact, queryFields);
        };
        query_in_edges_takes_fn = memoize(query_in_edges_takes_fn);

        reactans_coeff_fn = [=](string idReact) {
            return reactantsCoeff(idReact);
        };
        reactans_coeff_fn = memoize(reactans_coeff_fn);
    }

    auto getJsonRecord(string idRecord) -> string // id: "12234444:" format
    {
        dbvertex_all->GetRecord(idRecord.c_str());
        return dbvertex_all->GetJson();
    }

    auto getJsonRecordVertex(string idRecord) -> std::string // id: "12234444:" format
    {
        std::string json_str;
        dbvertex_all->GetRecord(idRecord.c_str());
        json_str = dbvertex_all->GetJson();
        return json_str;
    }

    auto getJsonRecordEdge(string idRecord) -> std::string // id: "12234444:" format
    {
        std::string json_str;
        dbedge_all->GetRecord(idRecord.c_str());
        json_str = dbedge_all->GetJson();
        return json_str;
    }

    auto queryRecord(string idRecord, vector<string> queryFields) -> string
    {
        //auto qrJson = dbvertex_all->idQuery(idRecord);
        //vector<string> resultRecord;
        //dbvertex_all->runQuery(qrJson, queryFields, resultRecord);
        //return resultRecord[0];
        dbvertex_all->GetRecord( (idRecord+":").c_str() );
        return dbvertex_all->GetJson();
    }

    auto loadRecord(string idRecord, vector<string> queryFields) -> jsonio::FieldSetMap
    {
        return dbvertex->loadRecordFields(idRecord, queryFields);
    }

    auto loadRecords(vector<string> idRecords) -> jsonio::ValuesTable
    {
        return dbvertex->loadRecords(idRecords, fieldPaths);
    }

    auto queryInEdgesDefines(string idSubst, vector<string> queryFields,  string level) -> vector<string>
    {
        string qrJson = "FOR e IN defines";
               qrJson += "\n FILTER e._to == '" + idSubst + "'";
               if (level != "-1")
                  qrJson += " && e.properties.level ==" + level;
               qrJson += "\n RETURN e";

        vector<string> resultsEdge;
        dbedge_all->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ),  queryFields, resultsEdge);
        return resultsEdge;
    }

    auto definesReactionSymbol(string idSubst, string level) -> std::string
    {
        string qrJson = "FOR v,e  IN 1..1 INBOUND '";
               qrJson += idSubst + "' \n defines\n";
               if (level != "-1")
                  qrJson += "FILTER e.properties.level ==" + level +"\n";
               qrJson += "RETURN v.properties.symbol";

        vector<string> resultsSymbols;
        dbedge_all->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ),  {}, resultsSymbols);
        if( resultsSymbols.empty() )
          return ""; //"undefined";
        return resultsSymbols[0];
    }

    auto queryInEdgesTakes(string idReact, vector<string> queryFields) -> vector<string>
    {
        auto queryin = dbedge_all->inEdgesQuery( "takes", idReact );
        vector<string> resultEdge;
        dbedge_all->runQuery( queryin,  queryFields, resultEdge );
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

        vector<string> resultsQuery;
        dbedge_all->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ),  {}, resultsQuery);

        for( auto result: resultsQuery )
        {
            auto domdata = jsonio::unpackJson( result );
            domdata->findKey("stoi_coeff", coeff);
            stoi_coeff = atof(coeff.c_str());
            domdata->findKey("symbol", symbol);
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

auto AbstractData::getJsonRecord(string idRecord) -> string
{
    return pimpl->get_json_record_fn_(idRecord);
}

auto AbstractData::getJsonRecordVertex(string idRecord) -> std::string
{
    return pimpl->get_json_bson_record_fn_v(idRecord);
}

auto AbstractData::getJsonRecordEdge(string idRecord) -> std::string
{
    return pimpl->get_json_bson_record_fn_e(idRecord);
}

auto AbstractData::queryRecord(string idRecord, vector<string> queryFields) -> string
{
    return pimpl->query_record_fn(idRecord, queryFields);
}
auto AbstractData::queryInEdgesDefines_(string idSubst, vector<string> queryFields,  string level) -> vector<string>
{
    return pimpl->query_in_edges_defines_fn(idSubst, queryFields, level);
}

auto AbstractData::definesReactionSymbol_(string idSubst, string level) -> std::string
{
    return pimpl->defines_reaction_symbol_fn(idSubst, level);
}

auto AbstractData::queryInEdgesTakes_(string idReact, vector<string> queryFields) -> vector<string>
{
    return pimpl->query_in_edges_takes_fn(idReact, queryFields);
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

bool AbstractData::recordExists(const string& id )
{
    return getDB()->Find( (id+":").c_str() );
}

auto AbstractData::addNewRecord( const jsonio::FieldSetMap& fldvalues, bool testValues ) -> string
{
    return getDB()->addNewVertex( getName(), fldvalues, testValues );
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
    vector<string> _resultData;
    auto queryin = pimpl->dbedge_all->inEdgesQuery( edgeCollections, idVertex );
    pimpl->dbedge_all->runQuery( queryin,  _queryFields, _resultData );

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
    vector<string> _resultData;
    auto queryin = pimpl->dbedge_all->inEdgesQuery( edgeCollections, idVertex );
    pimpl->dbedge_all->runQuery( queryin,  _queryFields, _resultData );

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
    vector<string> _resultData;

    pimpl->dbedge_all->runQuery( queryout,  _queryFields, _resultData );

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
    vector<string> _resultData;
    auto queryin = pimpl->dbedge_all->outEdgesQuery( edgeCollections, idVertex );
    pimpl->dbedge_all->runQuery( queryin,  _queryFields, _resultData );

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

auto AbstractData::setDefaultLevelForReactionDefinedSubst(jsonio::ValuesTable valuesTable) -> void
{
    for( const auto& subitem : valuesTable )
    {
        pimpl->substSymbolLevel[subitem[getDataName_DataIndex()["symbol"]]] = "0";
    }
}

auto AbstractData::getDataName_DataIndex() const -> std::map<std::string, int>
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

}
