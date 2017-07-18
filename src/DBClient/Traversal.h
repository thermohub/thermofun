#ifndef TRAVERSAL_H
#define TRAVERSAL_H

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"

namespace ThermoFun {

using  MapIdType = std::map<std::string, std::string>;
using  MapIdLevels = std::map<std::string, std::vector<std::string>>;
struct Database;

enum LevelMode {
    all= 0,
    single,
    multiple
};

///
/// \brief The Traversal class is used to make traversals through the database starting from a selected list of
///        vertexes
///
class Traversal
{
    boost::shared_ptr<bsonio::TDBGraph> dbgraph;

    std::map<std::string, std::string> definedSubstSymbolLevel;

    std::string level = "0";

    LevelMode levelMode = LevelMode::single;

    vector<string> queryIncomingEdgeDefines(std::string idSubst, vector<string> queryFields,  string level);
    vector<string> queryIncomingEdgeTakes(std::string idReact, vector<string> queryFields);
    vector<string> queryVertexReaction(std::string idReact, vector<string> queryFields);
    vector<string> queryVertexSubstance(std::string idSubst, vector<string> queryFields);


    // follows the incoming Defines edge for substance with _idSubst
    void followIncomingDefines(std::string _idSubst, MapIdType &result, string level_);
    // follows the incoming Takes edges for reaction with _idReac
    void followIncomingTakes(std::string _idReac, MapIdType &result);
    // returns the symbol of the reaction which defines the substance with _idSubst
    std::string getDefinesReactionSymbol(std::string _idSubst, string level_);
    // returns a map of reactants symbols and coeficients participating in the reaction
    std::map<std::string, double> getReactantsCoeffMap(std::string _idReac);

    // fills the map MapIdType with all vertexes connected to the vertex with id_
    void linkedDataFromId(std::string id_, MapIdType &result);

    Database getDatabase(MapIdType resultTraversal);

    public:

    ///
    /// \brief Traversal Constructor
    /// \param _dbgraph shared pointer for connection to the database
    ///
    explicit Traversal(boost::shared_ptr<bsonio::TDBGraph> _dbgraph);

    ///
    /// \brief getLinkedBsonFromSelectedData returns the linked data strting form the selected id list
    /// \param selNdx Indexes of the selected ids from aKeyList
    /// \param aKeyList List of ids
    /// \param level level of reactions
    /// \return a map of [id, vertex type]
    ///
    MapIdType getMapOfConnectedIds(vector<int> selNdx, vector<string> idsList, string level_);

    ///
    /// \brief getLinkedBsonFromIdList returns the linked data strting form the selected id list
    /// \param idList list of ids
    /// \param level level of reactions
    /// \return a map of [id, vertex type]
    ///
    MapIdType getMapOfConnectedIds(vector<string> idList, string level_);

    MapIdType getMapOfConnectedIds(vector<string> idList, std::map<std::string, std::string> substSymbolLevel_);

    MapIdType getMapOfConnectedIds(vector<string> idList);

    ///
    /// \brief getDatabaseFromTraversal parses a a map of [id, vertex type] into a ThermoFun Database object
    /// \param resultTraversal the map of [id, vertex type] resulted from traversal
    /// \param level level of reactions
    /// \return returns a ThermoFun Database object
    ///
    Database getDatabaseFromMapOfIds(MapIdType resultTraversal, string level_);
    Database getDatabaseFromMapOfIds(MapIdType resultTraversal, std::map<std::string, std::string> substSymbolLevel_);
};

}

#endif // TRAVERSAL_H

