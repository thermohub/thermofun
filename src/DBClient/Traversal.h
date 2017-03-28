

#ifndef TRAVERSAL_H
#define TRAVERSAL_H

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"

namespace ThermoFun {

using  MapIdBson = std::map<std::string, std::string>;
//struct Database;

class Traversal
{
    boost::shared_ptr<bsonio::TDBGraph> dbgraph;

    void followIncomingDefines(std::string _idSubst, MapIdBson &result);
    void followIncomingTakes(std::string _idReac, MapIdBson &result);

//    std::string getDefinesReactionSymbol(std::string _idSubst);
//    std::map<std::string, double> getReactantsCoeffMap(std::string _id);

    public:

    explicit Traversal(boost::shared_ptr<bsonio::TDBGraph> _dbgraph);
//    ~Traversal();

    MapIdBson getLinkedSelectedData(vector<int> selNdx, vector<string> aKeyList);

//    Database getDatabaseFromTraversal(MapIdBson resultTraversal);
};

}

#endif // TRAVERSAL_H

