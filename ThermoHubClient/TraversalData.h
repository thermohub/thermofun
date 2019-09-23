#ifndef GRAPHTRAVERSAL_H
#define GRAPHTRAVERSAL_H

// C++ includes
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <string>

namespace ThermoFun
{

class Database;
class SubstanceData_;
class ReactionData_;

using  VertexId_VertexType = std::map<std::string, std::string>;
using  VertexId_DefinesLevel = std::map<std::string, std::vector<std::string>>;

class TraversalData
{
public:

    TraversalData(SubstanceData_ *substData, ReactionData_ *reactData);

    auto operator=(TraversalData other) -> TraversalData&;

    virtual ~TraversalData();

    ///
    /// \brief getLinkedBsonFromSelectedData returns the linked data strting form the selected id list
    /// \param selNdx Indexes of the selected ids from aKeyList
    /// \param aKeyList List of ids
    /// \param level of edge defines
    /// \return a map of [id, vertex type]
    ///
    auto getMapOfConnectedIds(std::vector<std::size_t> selNdx, std::vector<std::string> idsList, std::string level_) -> VertexId_VertexType;

    ///
    /// \brief getLinkedBsonFromIdList returns a map of vertex ids and vertex type which is linked to the idList, for a given level
    /// \param idList list of ids
    /// \param level of edge defines
    /// \return a map of [vertex id, vertex type]
    ///
    auto getMapOfConnectedIds(std::vector<std::string> idList, std::string level_) -> VertexId_VertexType;

    /**
     * @brief getMapOfConnectedIds returns a map of vertex ids and vertex type which is linked to the idList, following the level specified for
     * individual substances defined by a reaction
     * @param idList list of vertex ids
     * @param substSymbolLevel_ map of substance symbol and level
     * @return a map of [vertex id, vertex type]
     */
    auto getMapOfConnectedIds(std::vector<std::string> idList, std::map<std::string, std::string> substSymbol_definesLevel) -> VertexId_VertexType;

    /**
     * @brief getMapOfConnectedIds returns a map of vertex ids and vertex type which is linked to the idList, for all levels
     * @param idList list of vertex ids
     * @return a map of [vertex id, vertex type]
     */
    auto getMapOfConnectedIds(std::vector<std::string> idList) -> VertexId_VertexType;

    /**
     * @brief getDatabase returns a ThermoFun Database from a pre selected list of ids or hub record keys
     * @param idList list of record keys
     * @return ThermoFun::Database object
     */
    auto getDatabase(const std::vector<std::string> &idList) -> Database;

    ///
    /// \brief getDatabaseFromTraversal returns a ThermoFun::Database object from a map of [vertex id, vertex type], for a given level
    /// \param resultTraversal the map of [id, vertex type] resulted from traversal (all connected vertexes)
    /// \param level of edge defines
    /// \return ThermoFun::Database object
    ///
    auto getDatabaseFromMapOfIds(VertexId_VertexType resultTraversal, std::string level_) -> Database;

    /**
     * @brief getDatabaseFromMapOfIds returns a ThermoFun::Database object from a map of [vertex id, vertex type], following the level specified for
     * individual substances defined by a reaction
     * @param resultTraversal the map of [id, vertex type] resulted from traversal (all connected vertexes)
     * @param substSymbolLevel_ map of substance symbol and level
     * @return ThermoFun::Database object
     */
    auto getDatabaseFromMapOfIds(VertexId_VertexType resultTraversal, std::map<std::string, std::string> substSymbol_definesLevel) -> Database;

    /**
     * @brief linkedDataIds sets the list of substsnce and reactio ids which are connected to a given list of database record keys
     * @param aKeyList record key list
     * @param substIds the list of substance ids resulted from the search
     * @param reactIds the list of reaction ids resulted from the search
     */
    auto linkedDataIds(const std::vector<std::string> aKeyList, std::vector<std::string> &substIds, std::vector<std::string> &reactIds) -> void;

private:

//    // follows the incoming Defines edge for substance with _idSubst
//    void followIncomingEdgeDefines(std::string idSubst, VertexId_VertexType &result, std::string level_);
//    // follows the incoming Takes edges for reaction with _idReac
//    void followIncomingEdgeTakes(std::string idReact, VertexId_VertexType &result);
//    // fills the map MapIdType with all vertexes connected to the vertex with id_
//    auto linkedDataFromId(std::string id_) -> VertexId_VertexType;
//    // returns the symbol of the reaction which defines the substance with _idSubst
//    auto level (std::string idSubst) -> std::string;
//    // returns a map of [id, vertex type] with all vertexes connected to the idList
//    auto getResult(std::vector<std::string> idList, std::vector<int> selNdx = {}) -> VertexId_VertexType;
//    // returns a ThermoFun::Database object from a map of [id, vertex type] resulted from traversal (all connected vertexes)
//    auto getDatabase(VertexId_VertexType resultTraversal) -> Database;

    //    auto getJsonRecord(string idRecord) -> string;

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};


}

#endif // GRAPHTRAVERSAL_H
