

#include "Traversal.h"

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
            result.insert(std::pair<std::string,std::string>(_id, valDB));
            if (_label == "substance")
            {
                 followIncomingDefines(_id, result);
            }
            if (_label == "reaction")
            {
                 followIncomingTakes(_id, result);
            }
        }
    }

    return result;
}

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
                result.insert((std::pair<std::string,std::string>(_idReac, _resultDataReac[0])));
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
                result.insert((std::pair<std::string,std::string>(_idSubst, _resultDataSubst[0])));
                followIncomingDefines(_idSubst, result);
            }
        }
    }

}


}


