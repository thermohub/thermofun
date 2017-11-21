#include "AbstractData.h"

// C++ includes
#include <functional>

// ThermoFun includes
#include "../OptimizationUtils.h"

using namespace bsonio;

namespace ThermoFun {

using QueryRecord           = std::function<string(string, vector<string>)>;


using QueryInEdgesDefines   = std::function<vector<string>(string, vector<string>, string)>;
using DefinesReactionSymbol = std::function<string(string, string)>;

using QueryInEdgesTakes     = std::function<vector<string>(string, vector<string>)>;
using ReactantsCoeff        = std::function<std::map<string, double>(string)>;

struct ThermoDataAbstract::Impl
{
    /// Vertex name
    const string name;
    /// Default query to vertex
    const string query;
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
    boost::shared_ptr<bsonio::TDBGraph> graphdb;

    /// Data base connection access to all data types
    boost::shared_ptr<bsonio::TDBGraph> graphdb_all;

    QueryRecord  query_record_fn;

    QueryInEdgesDefines   query_in_edges_defines_fn;
    DefinesReactionSymbol defines_reaction_symbol_fn;

    QueryInEdgesTakes   query_in_edges_takes_fn;
    ReactantsCoeff      reactans_coeff_fn;

    Impl(const string &name, const string &query, const vector<string> &paths, const vector<string> &headers, const vector<string> &names) :
         name(name), query(query), fieldPaths(paths), dataHeaders(headers), dataNames(names)
    {
        query_record_fn = [=](string idRecord, vector<string> queryFields) {
            return queryRecord(idRecord, queryFields);
        };

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

    auto queryRecord(string idRecord, vector<string> queryFields) -> string
    {
        string qrJson;
        qrJson = "{ \"_id\" : \"" + idRecord + "\"}";
        vector<string> resultRecord;
        graphdb->runQuery(qrJson, queryFields, resultRecord);
        return resultRecord[0];
    }

    auto queryInEdgesDefines(string idSubst, vector<string> queryFields,  string level) -> vector<string>
    {
        string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
        if (level != "-1")
        {
            qrJson += (idSubst + "', '$and' : [ { 'properties.level' : ");
            qrJson += level;
            qrJson += "}]}";
        } else
        {
            qrJson += (idSubst + "'}");
        }

        vector<string> resultsEdge;
        graphdb_all->runQuery(qrJson,  queryFields, resultsEdge);
        return resultsEdge;
    }

    auto definesReactionSymbol(string idSubst, string level) -> std::string
    {
        string symbol = ""; string idReact, _resultDataReac;
        bson record;
        vector<string> _resultDataEdge;
        vector<string> _queryFields = { "_outV", "_label"};

        _resultDataEdge = query_in_edges_defines_fn(idSubst, _queryFields, level);

        for(uint i = 0; i < _resultDataEdge.size(); i++)
        {
            jsonToBson(&record, _resultDataEdge[i]);
            bsonio::bson_to_key( record.data, "_outV", idReact );
            _resultDataReac = query_record_fn(idReact, {"_id", "_label", "properties.symbol"});

            jsonToBson(&record, _resultDataReac);
            bsonio::bson_to_key( record.data, "properties.symbol", symbol );
        }
        return symbol;
    }

    auto queryInEdgesTakes(string idReact, vector<string> queryFields) -> vector<string>
    {
        string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
        qrJson += (idReact + "' }");
        vector<string> resultEdge;
        graphdb_all->runQuery( qrJson,  queryFields, resultEdge );
        return resultEdge;
    }

    auto reactantsCoeff(string idReact) -> std::map<std::string, double>
    {
        std::map<std::string, double> reactantsCoeff;
        string coeff, symbol, idSubst, _resultDataSubst;
        double stoi_coeff;
        bson record;
        vector<string> _resultDataEdge;

        _resultDataEdge = query_in_edges_takes_fn(idReact, {"_outV", "properties.stoi_coeff"});

        for(uint i = 0; i < _resultDataEdge.size(); i++)
        {
            jsonToBson(&record, _resultDataEdge[i]);
            bsonio::bson_to_key( record.data, "properties.stoi_coeff", coeff );
            stoi_coeff = atof(coeff.c_str());

            bsonio::bson_to_key( record.data, "_outV", idSubst );
            _resultDataSubst = query_record_fn(idSubst, {"_id", "_label", "properties.symbol"});

            jsonToBson(&record, _resultDataSubst);
            bsonio::bson_to_key( record.data, "properties.symbol", symbol );
            reactantsCoeff.insert(std::pair<std::string,double>(symbol, stoi_coeff));
        }

        return reactantsCoeff;
    }
};

ThermoDataAbstract::ThermoDataAbstract(const string &name, const string &query, const vector<string> &paths, const vector<string> &headers, const vector<string> &names)
    : pimpl(new Impl(name, query, paths, headers, names))
{
    resetDataPathIndex();
}

ThermoDataAbstract::ThermoDataAbstract(const ThermoDataAbstract& other)
: pimpl(new Impl(*other.pimpl))
{}

ThermoDataAbstract::~ThermoDataAbstract()
{ }

auto ThermoDataAbstract::queryRecord(string idRecord, vector<string> queryFields) -> string
{
    return pimpl->query_record_fn(idRecord, queryFields);
}
auto ThermoDataAbstract::queryInEdgesDefines_(string idSubst, vector<string> queryFields,  string level) -> vector<string>
{
    return pimpl->query_in_edges_defines_fn(idSubst, queryFields, level);
}

auto ThermoDataAbstract::definesReactionSymbol_(string idSubst, string level) -> std::string
{
    return pimpl->defines_reaction_symbol_fn(idSubst, level);
}

auto ThermoDataAbstract::queryInEdgesTakes_(string idReact, vector<string> queryFields) -> vector<string>
{
    return pimpl->query_in_edges_takes_fn(idReact, queryFields);
}

auto ThermoDataAbstract::reactantsCoeff_(string idReact) -> std::map<std::string, double>
{
    return pimpl->reactans_coeff_fn(idReact);
}

auto ThermoDataAbstract::getName() const -> string
{
    return pimpl->name;
}

auto ThermoDataAbstract::getQuery() const -> string
{
    return pimpl->query;
}

auto ThermoDataAbstract::getDataFieldPaths() const -> vector<string>
{
    return pimpl->fieldPaths;
}

auto ThermoDataAbstract::setDataFieldPaths(const vector<string> &value) -> void
{
    pimpl->fieldPaths = value;
}

auto ThermoDataAbstract::getDataHeaders() const -> vector<string>
{
    return pimpl->dataHeaders;
}

auto ThermoDataAbstract::setDataHeaders(const vector<string> &value) -> void
{
    pimpl->dataHeaders = value;
}

auto ThermoDataAbstract::getDataNames() const -> vector<string>
{
    return pimpl->dataNames;
}

auto ThermoDataAbstract::setDataNames(const vector<string> &value) -> void
{
    pimpl->dataNames = value;
}

auto ThermoDataAbstract::getDB() const -> boost::shared_ptr<bsonio::TDBGraph>
{
    return pimpl->graphdb;
}

auto ThermoDataAbstract::setDB(const boost::shared_ptr<bsonio::TDBGraph> &value) -> void
{
    pimpl->graphdb     = value;
    pimpl->graphdb_all = value;
    pimpl->graphdb_all->resetMode(true);
}

auto ThermoDataAbstract::resetDataPathIndex() -> void
{
    pimpl->dataIndex.clear(); pimpl->dataPath.clear();
    for (uint i = 0; i<getDataNames().size(); i++)
    {
        pimpl->dataIndex[getDataNames()[i]] = i;
        pimpl->dataPath[getDataNames()[i]] = getDataFieldPaths()[i];
    }
}

auto ThermoDataAbstract::testElementsFormula( const string& aformula,
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

auto ThermoDataAbstract::getOutVertexIds( const string& edgeLabel, const string& idInVertex ) -> vector<string>
{
    vector<string> vertexIds_;
    string vertexId_;

    // select all EdgeTakes for reaction
    string queryJson = "{'_type': 'edge', '_label': '"
                     + edgeLabel  + "', '_inV': '"
                     + idInVertex + "' }";
    vector<string> _queryFields = { "_outV"};
    vector<string> _resultData;
    pimpl->graphdb->runQuery( queryJson,  _queryFields, _resultData );

    for( auto rec: _resultData)
    {
      vertexId_ =  bsonio::extractStringField( "_outV", rec );
      vertexIds_.push_back(vertexId_);
    }
    return vertexIds_;
}

auto ThermoDataAbstract::getOutVertexIds(const string& edgeLabel, const string& idInVertex,  vector<string> &edgeIds_) -> vector<string>
{
    vector<string> vertexIds_;
    string vertexId_, edgeId_;
    edgeIds_.clear();

    // select all EdgeTakes for reaction
    string queryJson = "{'_type': 'edge', '_label': '"
                     + edgeLabel  + "', '_inV': '"
                     + idInVertex + "' }";
    vector<string> _queryFields = { "_outV", "_id"};
    vector<string> _resultData;
    pimpl->graphdb->runQuery( queryJson,  _queryFields, _resultData );

    for( auto rec: _resultData)
    {
      vertexId_ =  bsonio::extractStringField( "_outV", rec );
      vertexIds_.push_back(vertexId_);
      edgeId_ =  bsonio::extractStringField( "_id", rec );
      edgeIds_.push_back(edgeId_);
    }
    return vertexIds_;
}

auto ThermoDataAbstract::setDefaultLevelForReactionDefinedSubst(bsonio::ValuesTable valuesTable) -> void
{
    for( const auto& subitem : valuesTable )
    {
        pimpl->substSymbolLevel[subitem[getDataName_DataIndex()["symbol"]]] = "0";
    }
}

auto ThermoDataAbstract::getDataName_DataIndex() const -> std::map<std::string, int>
{
    return pimpl->dataIndex;
}

auto ThermoDataAbstract::getDataName_DataFieldPath() const -> std::map<std::string, std::string>
{
    return pimpl->dataPath;
}

auto ThermoDataAbstract::getSubstSymbol_DefinesLevel() const -> std::map<std::string, std::string>
{
    return pimpl->substSymbolLevel;
}

auto ThermoDataAbstract::setSubstSymbol_DefinesLevel(const std::map<std::string, std::string> &value) -> void
{
    pimpl->substSymbolLevel = value;
}

auto ThermoDataAbstract::getDB_fullAccessMode() const -> boost::shared_ptr<bsonio::TDBGraph>
{
    return pimpl->graphdb_all;
}

}
