#include "GraphTraversal.h"

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"

// ThermoFun includes
#include "../Database.h"
#include "SubstanceData.h"
#include "ReactionData.h"

using namespace bsonio;

namespace ThermoFun
{

using GetJsonRecord           = std::function<string(string)>;

enum DefinesLevelMode {
    all= 0,
    single,
    multiple
};

struct GraphTraversal::Impl
{
    // Traversal
    std::map<std::string, std::string> definedSubstSymbolLevel;

    std::string level = "0";

    DefinesLevelMode levelMode = DefinesLevelMode::single;

    /// map of database ids and bson record of vertexes
    std::map<std::string, bson> idBsonRec;

    GetJsonRecord * get_json_record_fn;
    SubstanceData * substData;
    ReactionData  * reactData;

    Impl( )
    {
//        get_json_record_fn = [=](string idRecord) {
//            return getJsonRecord(idRecord);
//        };

    }

};

GraphTraversal::GraphTraversal(SubstanceData *substData, ReactionData *reactData) : pimpl(new Impl())
{
//    pimpl->get_json_record_fn = get_json_record_fn;
    pimpl->substData = substData;
    pimpl->reactData = reactData;
}

//auto GraphTraversal::operator=(GraphTraversal other) -> GraphTraversal&
//{
//    pimpl = std::move(other.pimpl);
//    return *this;
//}

GraphTraversal::~GraphTraversal()
{ }

auto GraphTraversal::level (std::string idSubst) -> std::string
{
    bson record;
    std::string level_;

//    switch(pimpl->levelMode)
//    {
//    case DefinesLevelMode::all         : level_ = "-1";  // follows all connected data
//        break;
//    case DefinesLevelMode::single      : level_ = pimpl->level;; // follows edges defines with level
//        break;
//    case DefinesLevelMode::multiple    : {
//        std::string substSymb; std::string key = idSubst +":";
//        std::string valDB = pimpl->get_json_record_fn(key);
//                jsonToBson( &record, valDB );
//        bsonio::bson_to_key( record.data, "properties.symbol", substSymb );
//        if (pimpl->definedSubstSymbolLevel.find(substSymb) != pimpl->definedSubstSymbolLevel.end()) // follows edges defines with specific leveles for substSymbols
//            level_ = pimpl->definedSubstSymbolLevel[substSymb];   // if the substance symbol is not found in the map, it uses the default level
//        else
//            level_ = pimpl->level;
//    }
//        break;
//    }

    return level_;
}

// Traversal

auto GraphTraversal::linkedDataFromId(std::string id_) -> MapId_VertexType
{
    string valDB, _idRecord, _label, _type, _symbol, level_;
    bson record;
    MapId_VertexType result;

    // get recrod
//    valDB = pimpl->getJsonRecord(id_);
    jsonToBson( &record, valDB );

    // Extract data from fields
    bsonio::bson_to_key( record.data, "_id",    _idRecord);
    bsonio::bson_to_key( record.data, "_label", _label);
    bsonio::bson_to_key( record.data, "_type",  _type);
    bsonio::bson_to_key( record.data, "properties.symbol",  _symbol);

    // get level mode
    level_ = level(_idRecord);

    // if not in the database proceed
    if (!result.count(_idRecord))
    {
        if (_label == "substance")
        {
            result.insert(std::pair<std::string,std::string>(_idRecord, "substance"));
            pimpl->idBsonRec[_idRecord] = record;
            followIncomingEdgeDefines(_idRecord, result, level_);
        }
        if (_label == "reaction")
        {
            result.insert(std::pair<std::string,std::string>(_idRecord, "reaction"));
            pimpl->idBsonRec[_idRecord] = record;
            followIncomingEdgeTakes(_idRecord, result);
        }
    }

    return result;
}

void GraphTraversal::followIncomingEdgeDefines(std::string _idSubst, MapId_VertexType &result, string level_)
{
//    string _idReact = "", _resultDataReac;
//    bson record;
//    vector<string> _resultDataEdge;

//    _resultDataEdge = pimpl->substData.queryInEdgesDefines(_idSubst, queryFieldsEdgeDefines, level_ );
//    for(uint i = 0; i < _resultDataEdge.size(); i++)
//    {
//        jsonToBson(&record, _resultDataEdge[i]);
//        bsonio::bson_to_key( record.data, "_outV", _idReact );
//        _resultDataReac = pimpl->reactData.queryRecord( _idReact, queryFieldsVertex);

//        jsonToBson(&record, _resultDataReac);

//        // if not in the database
//        if (!result.count(_idReact))
//        {
//            result.insert((std::pair<std::string,std::string>(_idReact, "reaction")));
//            pimpl->idBsonRec[_idReact] = record;
//            followIncomingEdgeTakes(_idReact, result);
//        }
//    }
}

void GraphTraversal::followIncomingEdgeTakes(std::string _idReact, MapId_VertexType &result)
{
//    string _idSubst = "", _resultDataSubst; string level_ = "0";
//    bson record;
//    vector<string> _resultDataEdge;

//    _resultDataEdge = pimpl->reactData.queryInEdgesTakes(_idReact, queryFieldsEdgeTakes);
//    for(uint i = 0; i < _resultDataEdge.size(); i++)
//    {
//        jsonToBson(&record, _resultDataEdge[i]);
//        bsonio::bson_to_key( record.data, "_outV", _idSubst );
//        _resultDataSubst = pimpl->substData.queryRecord(_idSubst, queryFieldsVertex);

//        jsonToBson(&record, _resultDataSubst);

//        // if not in the database
//        if (!result.count(_idSubst))
//        {
//            level_ = level(_idSubst);
//            result.insert((std::pair<std::string,std::string>(_idSubst, "substance")));
//            pimpl->idBsonRec[_idSubst] = record;
//            followIncomingEdgeDefines(_idSubst, result, level_);
//        }
//    }
}


}
