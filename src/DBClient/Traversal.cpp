
#include "Traversal.h"
#include "ReadFiles.h"
#include "Database.h"
#include "Substance.h"
#include "Reaction.h"
#include "Common/Exception.h"

using namespace bsonio;

namespace ThermoFun {

// Constructor
Traversal::Traversal (boost::shared_ptr<bsonio::TDBGraph> _dbgraph)
{
    dbgraph = _dbgraph;
    dbgraph->resetMode(true);
}

MapIdType Traversal::getMapOfConnectedIds(vector<int> selNdx, vector<string> idsList, string level_)
{
    level = level_;
    levelMode = LevelMode::single;

    MapIdType result;
    for( uint ii=0; ii<selNdx.size(); ii++ )
    {
        linkedDataFromId(idsList[selNdx[ii]], result);
    }
    return result;
}

MapIdType Traversal::getMapOfConnectedIds( vector<string> idList, string level_)
{
    level = level_;
    levelMode = LevelMode::single;

    MapIdType result;
    for (uint ii=0; ii<idList.size(); ii++)
    {
        linkedDataFromId(idList[ii], result);
    }
    return result;
}

MapIdType Traversal::getMapOfConnectedIds( vector<string> idList, std::map<std::string, std::string> substSymbolLevel_)
{
    MapIdType result;
    levelMode = LevelMode::multiple;

    if (!substSymbolLevel_.empty())
    {
        definedSubstSymbolLevel = substSymbolLevel_;
        for (uint ii=0; ii<idList.size(); ii++)
        {
            linkedDataFromId(idList[ii], result);
        }
    } else
    {
        levelMode = LevelMode::single;
        for (uint ii=0; ii<idList.size(); ii++)
        {
            linkedDataFromId(idList[ii], result);
        }
    }
    return result;
}

Database Traversal::getDatabaseFromMapOfIds(MapIdType resultTraversal, string level_)
{
    level = level_;
    levelMode = LevelMode::single;
    return getDatabase(resultTraversal);
}

Database Traversal::getDatabaseFromMapOfIds(MapIdType resultTraversal, std::map<std::string, std::string> substSymbolLevel_)
{
    if (!substSymbolLevel_.empty())
    {
        levelMode = LevelMode::multiple;
        definedSubstSymbolLevel = substSymbolLevel_;
    } else
    {
        levelMode = LevelMode::single;
    }
    return getDatabase(resultTraversal);
}

MapIdType Traversal::getMapOfConnectedIds( vector<string> idList )
{
    MapIdType result;
    levelMode = LevelMode::all;
    for (uint ii=0; ii<idList.size(); ii++)
    {
        linkedDataFromId(idList[ii], result);
    }
    return result;
}

void Traversal::linkedDataFromId(std::string id_, MapIdType &result)
{
    string valDB, _id, _label, _type, _symbol, level_;
    bson record;
    id_ = id_.c_str();
    dbgraph->GetRecord( id_.c_str() );
    valDB = dbgraph->GetJson();
    jsonToBson( &record, valDB );

    bsonio::bson_to_key( record.data, "_id",    _id);
    bsonio::bson_to_key( record.data, "_label", _label);
    bsonio::bson_to_key( record.data, "_type",  _type);
    bsonio::bson_to_key( record.data, "properties.symbol",  _symbol);

    switch(levelMode)
    {
    case LevelMode::all         : level_ = "-1";  // follows all connected data
        break;
    case LevelMode::single      : level_ = level;; // follows edges defines with level
        break;
    case LevelMode::multiple    : {
                                    if (definedSubstSymbolLevel.find(_symbol) != definedSubstSymbolLevel.end()) // follows edges defines with specific leveles for substSymbols
                                        level_ = definedSubstSymbolLevel[_symbol];   // if the substance symbol is not found in the map, it uses the default level
                                    else
                                        level_ = level;
                                  }
        break;
    }

    // if not in the database
    if (!result.count(_id))
    {
        if (_label == "substance")
        {
            result.insert(std::pair<std::string,std::string>(_id, "substance"));
            followIncomingDefines(_id, result, level_);
        }
        if (_label == "reaction")
        {
            result.insert(std::pair<std::string,std::string>(_id, "reaction"));
            followIncomingTakes(_id, result);
        }
    }

}

vector<string> Traversal::queryIncomingEdgeDefines(std::string idSubst, vector<string> queryFields, string level_)
{
    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
    if (level_ != "-1")
    {
        qrJson += (idSubst + "',");
        qrJson += "'$and' : [ { 'properties.level' : ";
        qrJson += level_;
        qrJson += "}]}";
    } else
    {
        qrJson += (idSubst + "'}");
    }

    vector<string> resultDataEdge;
    dbgraph->runQuery( qrJson,  queryFields, resultDataEdge );
    return resultDataEdge;
}

vector<string> Traversal::queryIncomingEdgeTakes(std::string idReact, vector<string> queryFields)
{
    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
    qrJson += (idReact+ "',");
    //    qrJson += "'$and' : [ { 'properties.level' : ";
    //    qrJson += level + "}]";
    qrJson += "}";
    vector<string> resultDataEdge;
    dbgraph->runQuery( qrJson,  queryFields, resultDataEdge );
    return resultDataEdge;
}

vector<string> Traversal::queryVertexReaction(std::string idReact, vector<string> queryFields)
{
    string qrJson;
    qrJson = "{ \"_id\" : \""+idReact+ "\"}";
    vector<string> resultDataReac;
    dbgraph->runQuery(qrJson, queryFields, resultDataReac);
    return resultDataReac;
}

vector<string> Traversal::queryVertexSubstance(std::string idSubst, vector<string> queryFields)
{
    string qrJson;
    qrJson = "{ \"_id\" : \""+idSubst+ "\"}";
    vector<string> resultDataReac;
    dbgraph->runQuery(qrJson, queryFields, resultDataReac);
    return resultDataReac;
}

void Traversal::followIncomingDefines(std::string _idSubst, MapIdType &result, string level_)
{
    string _idReac = ""; string kbuf = "";
    bson record;
    vector<string> _resultDataEdge, _resultDataReac;

    _resultDataEdge = queryIncomingEdgeDefines(_idSubst,{ "_outV", "_label"}, level_ );
    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", kbuf );
        _resultDataReac = queryVertexReaction(kbuf, {"_id", "_label", "properties.symbol"});

        if (_resultDataReac.size()>0)
        {
            jsonToBson(&record, _resultDataReac[0]);
            bsonio::bson_to_key( record.data, "_id", _idReac );

            // if not in the database
            if (!result.count(_idReac))
            {
                result.insert((std::pair<std::string,std::string>(_idReac, "reaction")));
                followIncomingTakes(_idReac, result);
            }
        }
    }
}

void Traversal::followIncomingTakes(std::string _idReac, MapIdType &result)
{
    string _idSubst = ""; string kbuf = ""; string level_ = "0";
    bson record;
    vector<string> _resultDataEdge, _resultDataSubst;

    _resultDataEdge = queryIncomingEdgeTakes(_idReac, { "_outV", "_label"});
    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", kbuf );
        _resultDataSubst = queryVertexSubstance(kbuf, {"_id", "_label", "properties.symbol"});
        for (auto dataSubst :_resultDataSubst)
        {
            jsonToBson(&record, dataSubst);
            bsonio::bson_to_key( record.data, "_id", _idSubst );

            // if not in the database
            if (!result.count(_idSubst))
            {
                switch(levelMode)
                {
                case LevelMode::all         : level_ = "-1";  // follows all connected data
                    break;
                case LevelMode::single      : level_ = level;; // follows edges defines with level
                    break;
                case LevelMode::multiple    : {
                    std::string substSymb; std::string key = _idSubst +":";
                    dbgraph->GetRecord( key.c_str() );
                    std::string valDB = dbgraph->GetJson();
                    jsonToBson( &record, valDB );
                    bsonio::bson_to_key( record.data, "properties.symbol", substSymb );
                    if (definedSubstSymbolLevel.find(substSymb) != definedSubstSymbolLevel.end()) // follows edges defines with specific leveles for substSymbols
                        level_ = definedSubstSymbolLevel[substSymb];   // if the substance symbol is not found in the map, it uses the default level
                    else
                        level_ = level;
                }
                    break;
                }
                result.insert((std::pair<std::string,std::string>(_idSubst, "substance")));
                followIncomingDefines(_idSubst, result, level_);
            }
        }
    }
}

Database Traversal::getDatabase(MapIdType resultTraversal)
{
    string key, valDB, _idSubst, substSymb; string level_ = level;
    bson record;
    Database tdb;
    auto map_ = resultTraversal;

    // The set of all aqueous species in the database
    SubstancesMap substances_map;
    // The set of all gaseous species in the database
    ReactionsMap reactions_map;

    // get substances and the reaction symbol if necessary
    for(auto iterator = map_.begin(); iterator != map_.end(); iterator++)
    {
        if (iterator->second == "substance")
        {
            key = iterator->first +":";
            dbgraph->GetRecord( key.c_str() );
            valDB = dbgraph->GetJson();
            jsonToBson( &record, valDB );
            bsonio::bson_to_key( record.data, "_id", _idSubst );
            bsonio::bson_to_key( record.data, "properties.symbol", substSymb );

            switch(levelMode)
            {
            case LevelMode::all         : level_ = "-1";  // follows all connected data
                break;
            case LevelMode::single      : level_ = level;; // follows edges defines with level
                break;
            case LevelMode::multiple    : {
                if (definedSubstSymbolLevel.find(substSymb) != definedSubstSymbolLevel.end()) // follows edges defines with specific leveles for substSymbols
                    level_ = definedSubstSymbolLevel[substSymb];   // if the substance symbol is not found in the map, it uses the default level
                else
                    level_ = level;
                }
                break;
            }
            Substance substance = parseSubstance(record.data);

            // get reaction symbol which define substance with _idSubst
            string definesReactSymb = getDefinesReactionSymbol(_idSubst, level_);
            if (definesReactSymb != "")
            {
                substance.setReactionSymbol(definesReactSymb);
                substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
            }

            if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
                substances_map[substance.symbol()] = substance;
            } else {
                errorSameSymbol("substance", substance.symbol(), __LINE__, __FILE__ );
            }
        } else
            if (iterator->second == "reaction")
            {
                key = iterator->first +":";

                dbgraph->GetRecord( key.c_str() );
                valDB = dbgraph->GetJson();
                jsonToBson( &record, valDB );
                bsonio::bson_to_key( record.data, "_id", key );

                Reaction reaction = ThermoFun::parseReaction(record.data);

                // get reactants by following reaction incoming takes edge
                reaction.setReactants(getReactantsCoeffMap(key));

                if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
                    reactions_map[reaction.symbol()] = reaction;
                } else {
                    errorSameSymbol("reaction", reaction.symbol(), __LINE__, __FILE__ );
                }
            }
    }
    tdb.addMapReactions(reactions_map);
    tdb.addMapSubstances(substances_map);
    return tdb;
}

std::string Traversal::getDefinesReactionSymbol(std::string _idSubst, string level_)
{
    string kbuf = "";
    bson record;
    vector<string> _queryFields = { "_outV", "_label"};
    vector<string> _resultDataEdge, _resultDataReac;

    _resultDataEdge = queryIncomingEdgeDefines(_idSubst, _queryFields, level_);

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", kbuf );
        _resultDataReac = queryVertexReaction(kbuf, {"_id", "_label", "properties.symbol"});
        if (_resultDataReac.size()>0)
        {
            jsonToBson(&record, _resultDataReac[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
        }
    }
    return kbuf;
}

std::map<std::string, double> Traversal::getReactantsCoeffMap(std::string _idReac)
{
    std::map<std::string, double> map;
    string kbuf;
    double stoi_coeff;
    bson record;
    vector<string> _resultDataEdge, _resultDataSubst;

    _resultDataEdge = queryIncomingEdgeTakes(_idReac, { "_outV", "properties.stoi_coeff"});

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "properties.stoi_coeff", kbuf );
        stoi_coeff = atof(kbuf.c_str());

        bsonio::bson_to_key( record.data, "_outV", kbuf );
        _resultDataSubst = queryVertexSubstance(kbuf,{"_id", "_label", "properties.symbol"});

        if (_resultDataSubst.size()>0)
        {
            jsonToBson(&record, _resultDataSubst[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
        }

        map.insert(std::pair<std::string,double>(kbuf,stoi_coeff));
    }

    dbgraph->resetMode(true);
    return map;
}

}


