
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
}

MapIdBson Traversal::getLinkedBsonFromSelectedData(vector<int> selNdx, vector<string> aKeyList, string level)
{
    MapIdBson result;
    for( uint ii=0; ii<selNdx.size(); ii++ )
    {
        linkedBsonDataFromId(aKeyList[selNdx[ii]], result, level);
    }
    return result;
}

MapIdBson Traversal::getLinkedBsonFromIdList( vector<string> idList, string level)
{
    MapIdBson result;
    for (uint ii=0; ii<idList.size(); ii++)
    {
        linkedBsonDataFromId(idList[ii], result, level);
    }
    return result;
}

void Traversal::linkedBsonDataFromId(std::string id_, MapIdBson &result, string level)
{
    string valDB, _id, _label, _type, _level, _symbol;
    bson record;
    id_ = id_.c_str();
    dbgraph->GetRecord( id_.c_str() );
    valDB = dbgraph->GetJson();
    jsonToBson( &record, valDB );

    bsonio::bson_to_key( record.data, "_id",    _id);
    bsonio::bson_to_key( record.data, "_label", _label);
    bsonio::bson_to_key( record.data, "_type",  _type);
    bsonio::bson_to_key( record.data, "properties.level",  _level);
    bsonio::bson_to_key( record.data, "properties.symbol",  _symbol);

    // if not in the database
    if (!result.count(_id))
    {
        if (_label == "substance")
        {
            result.insert(std::pair<std::string,std::string>(_id, "substance"));
            followIncomingDefines(_id, result, level);
        }
        if (_label == "reaction")
        {
            // Check if levels match
            if (level !=  _level)
            {
                errorNotMatchingLevel(_label, (_symbol + " level " + _level + " traversal level " + level), __LINE__, __FILE__);// error level of traversal
            }
            result.insert(std::pair<std::string,std::string>(_id, "reaction"));
            followIncomingTakes(_id, result, level);
        }
    }

}

Database Traversal::getDatabaseFromTraversal(MapIdBson resultTraversal, string level)
{
    string key, valDB, _idSubst;
    bson record;
    Database tdb;
    auto map_ = resultTraversal;

    // The set of all aqueous species in the database
    SubstancesMap substances_map;
    // The set of all gaseous species in the database
    ReactionsMap reactions_map;

    dbgraph->resetMode(true);

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

            Substance substance = parseSubstance(record.data);

            // get reaction symbol which define substance with _idSubst
            if (getDefinesReactionSymbol(_idSubst, level) != "")
            {
                substance.setReactionSymbol(getDefinesReactionSymbol(_idSubst, level));
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
                reaction.setReactants(getReactantsCoeffMap(key, level));

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

void Traversal::followIncomingDefines(std::string _idSubst, MapIdBson &result, string level)
{
    string _idReac = ""; string kbuf = "";
    bson record;
//    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
//    qrJson += _idSubst;
//    qrJson += "' }";
    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
    qrJson += (_idSubst + "',");
    qrJson += "'$and' : [ { 'properties.level' : ";
    qrJson += level;
    qrJson += "}]}";

    vector<string> _queryFields = { "_outV", "_label"};
    vector<string> _resultDataEdge, _resultDataReac;
    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataReac);

        if (_resultDataReac.size()>0)
        {
            jsonToBson(&record, _resultDataReac[0]);
            bsonio::bson_to_key( record.data, "_id", _idReac );

            // if not in the database
            if (!result.count(_idReac))
            {
                result.insert((std::pair<std::string,std::string>(_idReac, "reaction")));
                followIncomingTakes(_idReac, result, level);
            }
        }
    }
}

void Traversal::followIncomingTakes(std::string _idReac, MapIdBson &result, string level)
{
    string _idSubst = ""; string kbuf = "";
    bson record;
//    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
//    qrJson += _idReac;
//    qrJson += "' }";
    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
    qrJson += (_idReac + "',");
    qrJson += "'$and' : [ { 'properties.level' : ";
    qrJson += level;
    qrJson += "}]}";

    vector<string> _queryFields = { "_outV", "_label"};
    vector<string> _resultDataEdge, _resultDataSubst;
    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataSubst);

        if (_resultDataSubst.size()>0)
        {
            jsonToBson(&record, _resultDataSubst[0]);
            bsonio::bson_to_key( record.data, "_id", _idSubst );

            // if not in the database
            if (!result.count(_idSubst))
            {
                result.insert((std::pair<std::string,std::string>(_idSubst, "substance")));
                followIncomingDefines(_idSubst, result, level);
            }
        }
    }

}

std::string Traversal::getDefinesReactionSymbol(std::string _idSubst, string level)
{
    string kbuf = "";
    bson record;
//    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
//    qrJson += _idSubst;
//    qrJson += "' }";
    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
    qrJson += (_idSubst + "',");
    qrJson += "'$and' : [ { 'properties.level' : ";
    qrJson += level;
    qrJson += "}]}";

    vector<string> _queryFields = { "_outV", "_label"};
    vector<string> _resultDataEdge, _resultDataReac;
    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataReac);

        if (_resultDataReac.size()>0)
        {
            jsonToBson(&record, _resultDataReac[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
        }
    }
    return kbuf;
}

std::map<std::string, double> Traversal::getReactantsCoeffMap(std::string _id, string level)
{
    std::map<std::string, double> map;
    string kbuf;
    double stoi_coeff;
    bson record;
//    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
//    qrJson += _id;
//    qrJson += "' }";
    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
    qrJson += (_id+ "',");
    qrJson += "'$and' : [ { 'properties.level' : ";
    qrJson += level;
    qrJson += "}]}";

    vector<string> _queryFields = { "_outV", "properties.stoi_coeff"};
    vector<string> _resultDataEdge, _resultDataSubst;
    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "properties.stoi_coeff", kbuf );
        stoi_coeff = atof(kbuf.c_str());

        bsonio::bson_to_key( record.data, "_outV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataSubst);

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


