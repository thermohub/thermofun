
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
    string valDB, _idRecord, _label, _type, _symbol, level_;
    bson record;
    id_ = id_.c_str();

    // get recrod
    dbgraph->GetRecord( id_.c_str() );
    valDB = dbgraph->GetJson();
    jsonToBson( &record, valDB );

    // Extract data from fields
    bsonio::bson_to_key( record.data, "_id",    _idRecord);
    bsonio::bson_to_key( record.data, "_label", _label);
    bsonio::bson_to_key( record.data, "_type",  _type);
    bsonio::bson_to_key( record.data, "properties.symbol",  _symbol);

    // get level mode
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

    // if not in the database proceed
    if (!result.count(_idRecord))
    {
        if (_label == "substance")
        {
            result.insert(std::pair<std::string,std::string>(_idRecord, "substance"));
            idBsonRec[_idRecord] = record;
            followIncomingDefines(_idRecord, result, level_);
        }
        if (_label == "reaction")
        {
            result.insert(std::pair<std::string,std::string>(_idRecord, "reaction"));
            idBsonRec[_idRecord] = record;
            followIncomingTakes(_idRecord, result);
        }
    }

}

vector<string> Traversal::queryIncomingEdgeDefines(std::string idSubst, vector<string> queryFields, string level_)
{
    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
    if (level_ != "-1")
    {
        qrJson += (idSubst + "', '$and' : [ { 'properties.level' : ");
        qrJson += level_;
        qrJson += "}]}";
    } else
    {
        qrJson += (idSubst + "'}");
    }

    vector<string> resultsEdge;
    dbgraph->runQuery( qrJson,  queryFields, resultsEdge );
    return resultsEdge;
}

vector<string> Traversal::queryIncomingEdgeTakes(std::string idReact, vector<string> queryFields)
{
    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
    qrJson += (idReact + "' }");
    vector<string> resultEdge;
    dbgraph->runQuery( qrJson,  queryFields, resultEdge );
    return resultEdge;
}

vector<string> Traversal::queryVertexReaction(std::string idReact, vector<string> queryFields)
{
    string qrJson;
    qrJson = "{ \"_id\" : \"" + idReact + "\"}";
    vector<string> resultReact;
    dbgraph->runQuery(qrJson, queryFields, resultReact);
    return resultReact;
}

vector<string> Traversal::queryVertexSubstance(std::string idSubst, vector<string> queryFields)
{
    string qrJson;
    qrJson = "{ \"_id\" : \"" + idSubst + "\"}";
    vector<string> resultSubst;
    dbgraph->runQuery(qrJson, queryFields, resultSubst);
    return resultSubst;
}

void Traversal::followIncomingDefines(std::string _idSubst, MapIdType &result, string level_)
{
    string _idReact = "";
    bson record;
    vector<string> _resultDataEdge, _resultDataReac;

    _resultDataEdge = queryIncomingEdgeDefines(_idSubst,{ "_outV", "_label"}, level_ );
    idSubstDefinesEdges[_idSubst] = _resultDataEdge;
    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", _idReact );
        _resultDataReac = queryVertexReaction(_idReact, {"_id", "_label", "properties.symbol"});
        idDefinesOutReact[_idReact] = _resultDataReac;

        if (_resultDataReac.size()>0)
        {
            jsonToBson(&record, _resultDataReac[0]);

            // if not in the database
            if (!result.count(_idReact))
            {
                result.insert((std::pair<std::string,std::string>(_idReact, "reaction")));
                idBsonRec[_idReact] = record;
                followIncomingTakes(_idReact, result);
            }
        }
    }
}

void Traversal::followIncomingTakes(std::string _idReact, MapIdType &result)
{
    string _idSubst = ""; string level_ = "0";
    bson record;
    vector<string> _resultDataEdge, _resultDataSubst;

    _resultDataEdge = queryIncomingEdgeTakes(_idReact, { "_outV", "_label", "properties.stoi_coeff" });
    idReactTakesEdges[_idReact] = _resultDataEdge;
    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", _idSubst );
        _resultDataSubst = queryVertexSubstance(_idSubst, {"_id", "_label", "properties.symbol"});
        idTakesOutSubst[_idSubst] = _resultDataSubst;
        for (auto dataSubst :_resultDataSubst)
        {
            jsonToBson(&record, dataSubst);

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
                idBsonRec[_idSubst] = record;
                followIncomingDefines(_idSubst, result, level_);
            }
        }
    }
}

Database Traversal::getDatabase(MapIdType resultTraversal)
{
    string _idSubst, _idReact, substSymb; string level_ = level;
    bson record;
    Database tdb;
    auto result = resultTraversal;

    // The set of all aqueous species in the database
    SubstancesMap substances_map;
    // The set of all gaseous species in the database
    ReactionsMap reactions_map;

    // get substances and the reaction symbol if necessary
    for(auto iterator = result.begin(); iterator != result.end(); iterator++)
    {
        if (iterator->second == "substance")
        {
            _idSubst = iterator->first;
            record = idBsonRec[_idSubst];
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
            string definesReactSymb = getDefinesReactionSymbol(_idSubst);
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
                _idReact = iterator->first;
                record = idBsonRec[_idReact];

                Reaction reaction = ThermoFun::parseReaction(record.data);

                // get reactants by following reaction incoming takes edge
                reaction.setReactants(getReactantsCoeffMap(_idReact));

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

std::string Traversal::getDefinesReactionSymbol(std::string _idSubst)
{
    string symbol = ""; string _idReact;
    bson record;
    vector<string> _resultDataEdge, _resultDataReac;

    _resultDataEdge = idSubstDefinesEdges[_idSubst];

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", _idReact );
        _resultDataReac = idDefinesOutReact[_idReact];
        if (_resultDataReac.size()>0)
        {
            jsonToBson(&record, _resultDataReac[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", symbol );
        }
    }
    return symbol;
}

std::map<std::string, double> Traversal::getReactantsCoeffMap(std::string _idReac)
{
    std::map<std::string, double> reactantsCoeff;
    string coeff, symbol, _idSubst;
    double stoi_coeff;
    bson record;
    vector<string> _resultDataEdge, _resultDataSubst;

    _resultDataEdge = idReactTakesEdges[_idReac];

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "properties.stoi_coeff", coeff );
        stoi_coeff = atof(coeff.c_str());

        bsonio::bson_to_key( record.data, "_outV", _idSubst );
        _resultDataSubst = idTakesOutSubst[_idSubst];

        if (_resultDataSubst.size()>0)
        {
            jsonToBson(&record, _resultDataSubst[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", symbol );
        }
        reactantsCoeff.insert(std::pair<std::string,double>(symbol, stoi_coeff));
    }
    dbgraph->resetMode(true);
    return reactantsCoeff;
}

}


