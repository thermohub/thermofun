#ifndef GRAPHTRAVERSAL_H
#define GRAPHTRAVERSAL_H

// C++ includes
#include <memory>
#include <vector>
#include <map>
#include <functional>

namespace ThermoFun
{

struct Database;
struct SubstanceData;
struct ReactionData;

using  MapId_VertexType = std::map<std::string, std::string>;
using  MapId_DefinesLevels = std::map<std::string, std::vector<std::string>>;

std::vector<std::string> queryFieldsVertex       = {"_id", "_label", "properties.symbol"};
std::vector<std::string> queryFieldsEdgeDefines  = { "_outV", "_label"};
std::vector<std::string> queryFieldsEdgeTakes    = { "_outV", "_label", "properties.stoi_coeff" };

class GraphTraversal
{
public:

    GraphTraversal(SubstanceData *substData, ReactionData *reactData);

//    auto operator=(GraphTraversal other) -> GraphTraversal&;

    virtual ~GraphTraversal();

    auto getDatabase(MapId_VertexType resultTraversal) const -> Database;

private:

    // follows the incoming Defines edge for substance with _idSubst
    void followIncomingEdgeDefines(std::string idSubst, MapId_VertexType &result, std::string level_);
    // follows the incoming Takes edges for reaction with _idReac
    void followIncomingEdgeTakes(std::string idReact, MapId_VertexType &result);
    // fills the map MapIdType with all vertexes connected to the vertex with id_
    auto linkedDataFromId(std::string id_) -> MapId_VertexType;
    // returns the symbol of the reaction which defines the substance with _idSubst
    auto level (std::string idSubst) -> std::string;

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};


}

#endif // GRAPHTRAVERSAL_H
