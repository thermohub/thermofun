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

class TraversalData
{
public:

    TraversalData(SubstanceData *substData, ReactionData *reactData);

    auto operator=(TraversalData other) -> TraversalData&;

    /// Construct a copy of an TraversalData instance
//    TraversalData(const TraversalData& other);

    virtual ~TraversalData();

    ///
    /// \brief getLinkedBsonFromSelectedData returns the linked data strting form the selected id list
    /// \param selNdx Indexes of the selected ids from aKeyList
    /// \param aKeyList List of ids
    /// \param level level of reactions
    /// \return a map of [id, vertex type]
    ///
    auto getMapOfConnectedIds(std::vector<int> selNdx, std::vector<std::string> idsList, std::string level_) -> MapId_VertexType;

    ///
    /// \brief getLinkedBsonFromIdList returns the linked data strting form the selected id list
    /// \param idList list of ids
    /// \param level level of reactions
    /// \return a map of [id, vertex type]
    ///
    auto getMapOfConnectedIds(std::vector<std::string> idList, std::string level_) -> MapId_VertexType;

    auto getMapOfConnectedIds(std::vector<std::string> idList, std::map<std::string, std::string> substSymbolLevel_) -> MapId_VertexType;

    auto getMapOfConnectedIds(std::vector<std::string> idList) -> MapId_VertexType;

    ///
    /// \brief getDatabaseFromTraversal parses a a map of [id, vertex type] into a ThermoFun Database object
    /// \param resultTraversal the map of [id, vertex type] resulted from traversal
    /// \param level level of reactions
    /// \return returns a ThermoFun Database object
    ///
    auto getDatabaseFromMapOfIds(MapId_VertexType resultTraversal, std::string level_) -> Database;
    auto getDatabaseFromMapOfIds(MapId_VertexType resultTraversal, std::map<std::string, std::string> substSymbolLevel_) -> Database;

private:

    // follows the incoming Defines edge for substance with _idSubst
    void followIncomingEdgeDefines(std::string idSubst, MapId_VertexType &result, std::string level_);
    // follows the incoming Takes edges for reaction with _idReac
    void followIncomingEdgeTakes(std::string idReact, MapId_VertexType &result);
    // fills the map MapIdType with all vertexes connected to the vertex with id_
    auto linkedDataFromId(std::string id_) -> MapId_VertexType;
    // returns the symbol of the reaction which defines the substance with _idSubst
    auto level (std::string idSubst) -> std::string;

    auto getResult(std::vector<std::string> idList, std::vector<int> selNdx = {}) -> MapId_VertexType;

    auto getDatabase(MapId_VertexType resultTraversal) -> Database;

    //    /**
    //     * @brief getJsonRecord
    //     * @param idRecord
    //     * @return
    //     */
    //    auto getJsonRecord(string idRecord) -> string;

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};


}

#endif // GRAPHTRAVERSAL_H
