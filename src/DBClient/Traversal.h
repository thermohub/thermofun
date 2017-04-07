

#ifndef TRAVERSAL_H
#define TRAVERSAL_H

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"

namespace ThermoFun {

using  MapIdBson = std::map<std::string, std::string>;
struct Database;

class Traversal
{
    boost::shared_ptr<bsonio::TDBGraph> dbgraph;

    void followIncomingDefines(std::string _idSubst, MapIdBson &result, string level);
    void followIncomingTakes(std::string _idReac, MapIdBson &result, string level);

    std::string getDefinesReactionSymbol(std::string _idSubst, string level);
    std::map<std::string, double> getReactantsCoeffMap(std::string _id, string level);

    void linkedBsonDataFromId(std::string id_, MapIdBson &result, string level);

    public:

    explicit Traversal(boost::shared_ptr<bsonio::TDBGraph> _dbgraph);
//    ~Traversal();

    MapIdBson getLinkedBsonFromSelectedData(vector<int> selNdx, vector<string> aKeyList, string level);
    MapIdBson getLinkedBsonFromIdList(vector<string> idList, string level);

    Database getDatabaseFromTraversal(MapIdBson resultTraversal, string level);
};

}

#endif // TRAVERSAL_H

