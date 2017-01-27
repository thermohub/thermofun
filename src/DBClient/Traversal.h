#ifdef DBCLIENT_H

#ifndef TRAVERSAL_H
#define TRAVERSAL_H

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"

namespace ThermoFun {

using MapIdBson = std::map<std::string, bson>;

class Traversal
{
    boost::shared_ptr<bsonio::TDBGraph> dbgraph;

    void followIncomingDefines(std::string _idSubst, MapIdBson &result);
    void followIncomingTakes(std::string _idReac, MapIdBson &result);

    public:

    explicit Traversal(boost::shared_ptr<bsonio::TDBGraph> _dbgraph);
//    ~Traversal();

    MapIdBson getLinkedSelectedData(vector<int> selNdx, vector<string> aKeyList);
};

}

#endif // TRAVERSAL_H

#endif
