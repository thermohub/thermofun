
#include "Traversal.h"
//#include "ReadFiles.h"
//#include "Database.h"

using namespace bsonio;

namespace ThermoFun {

// Constructor
Traversal::Traversal (boost::shared_ptr<bsonio::TDBGraph> _dbgraph)
{
    dbgraph = _dbgraph;
}

MapIdBson Traversal::getLinkedSelectedData( vector<int> selNdx, vector<string> aKeyList)
{
    string key, valDB, _id, _label, _type;

    MapIdBson result;

    for( uint ii=0; ii<selNdx.size(); ii++ )
    {
        bson record;
        key = aKeyList[selNdx[ii]];
        dbgraph->GetRecord( key.c_str() );
        valDB = dbgraph->GetJson();
        jsonToBson( &record, valDB );

        bsonio::bson_to_key( record.data, "_id",    _id);
        bsonio::bson_to_key( record.data, "_label", _label);
        bsonio::bson_to_key( record.data, "_type",  _type);

        // if not in the database
        if (!result.count(_id))
        {
            if (_label == "substance")
            {
                result.insert(std::pair<std::string,std::string>(_id, "substance"));
                followIncomingDefines(_id, result);
            }
            if (_label == "reaction")
            {
                result.insert(std::pair<std::string,std::string>(_id, "reaction"));
                followIncomingTakes(_id, result);
            }
        }
    }

    return result;
}

//Database Traversal::getDatabaseFromTraversal(MapIdBson resultTraversal)
//{
//    string key, valDB, _idSubst;
//    bson record;
//    Database tdb;
//    auto map_ = resultTraversal;

//    // The set of all aqueous species in the database
//    SubstancesMap substances_map;
//    // The set of all gaseous species in the database
//    ReactionsMap reactions_map;

//    // get substances and the reaction symbol if necessary
//    for(auto iterator = map_.begin(); iterator != map_.end(); iterator++)
//    {
//        if (iterator->second == "substance")
//        {
//            key = iterator->first +":";
//            dbgraph->GetRecord( key.c_str() );
//            valDB = dbgraph->GetJson();
//            jsonToBson( &record, valDB );
//            bsonio::bson_to_key( record.data, "_id", _idSubst );

//            Substance substance = parseSubstance(record.data);

//            if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
//                substances_map[substance.symbol()] = substance;
//            } else {
//                // ERROR substance with the same symbol found!
//            }

//            // get reaction symbol which define substance with _idSubst
//            if (getDefinesReactionSymbol(_idSubst) != "")
//            {
//                substance.setReactionSymbol(getDefinesReactionSymbol(_idSubst));
//                substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
//            }

//            substances_map[substance.symbol()] = substance;
//        } else
//            if (iterator->second == "reaction")
//            {
//                key = iterator->first +":";
//                dbgraph->GetRecord( key.c_str() );
//                valDB = dbgraph->GetJson();
//                jsonToBson( &record, valDB );
//                bsonio::bson_to_key( record.data, "_id", key );

//                ThermoFun::Reaction reaction = ThermoFun::parseReaction(record.data);

//                if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
//                    reactions_map[reaction.symbol()] = reaction;
//                } else {
//                    // ERROR reaction with the same symbol found!
//                }

//                // get reactants by following reaction incoming takes edge
//                reaction.setReactants(getReactantsCoeffMap(key));

//                reactions_map[reaction.symbol()] = reaction;
//            }
//    }
//    return tdb;
//}

void Traversal::followIncomingDefines(std::string _idSubst, MapIdBson &result)
{
    string _idReac = ""; string kbuf = "";
    bson record;
    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
    qrJson += _idSubst;
    qrJson += "' }";

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
                followIncomingTakes(_idReac, result);
            }
        }
    }
}

void Traversal::followIncomingTakes(std::string _idReac, MapIdBson &result)
{
    string _idSubst = ""; string kbuf = "";
    bson record;
    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
    qrJson += _idReac;
    qrJson += "' }";

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
                followIncomingDefines(_idSubst, result);
            }
        }
    }

}

//std::string Traversal::getDefinesReactionSymbol(std::string _idSubst)
//{
//    string kbuf = "";
//    bson record;
//    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
//    qrJson += _idSubst;
//    qrJson += "' }";

//    vector<string> _queryFields = { "_outV", "_label"};
//    vector<string> _resultDataEdge, _resultDataReac;
//    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

//    for(uint i = 0; i < _resultDataEdge.size(); i++)
//    {
//        jsonToBson(&record, _resultDataEdge[i]);
//        bsonio::bson_to_key( record.data, "_outV", kbuf );
//        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
//        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataReac);

//        if (_resultDataReac.size()>0)
//        {
//            jsonToBson(&record, _resultDataReac[0]);
//            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
//        }
//    }
//    return kbuf;
//}

//std::map<std::string, double> Traversal::getReactantsCoeffMap(std::string _id)
//{
//    std::map<std::string, double> map;
//    string kbuf;
//    double stoi_coeff;
//    bson record;
//    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
//    qrJson += _id;
//    qrJson += "' }";

//    vector<string> _queryFields = { "_outV", "properties.stoi_coeff"};
//    vector<string> _resultDataEdge, _resultDataSubst;
//    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

//    for(uint i = 0; i < _resultDataEdge.size(); i++)
//    {
//        jsonToBson(&record, _resultDataEdge[i]);
//        bsonio::bson_to_key( record.data, "properties.stoi_coeff", kbuf );
//        stoi_coeff = atof(kbuf.c_str());

//        bsonio::bson_to_key( record.data, "_outV", kbuf );
//        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
//        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataSubst);

//        if (_resultDataSubst.size()>0)
//        {
//            jsonToBson(&record, _resultDataSubst[0]);
//            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
//        }

//        map.insert(std::pair<std::string,double>(kbuf,stoi_coeff));
//    }

//    return map;
//}


}


