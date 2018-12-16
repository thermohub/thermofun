#ifndef DATABASECLIENT_H
#define DATABASECLIENT_H

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "jsonio/dbconnect.h"

namespace ThermoFun
{

class Database;
//class Reaction;
//class Substance;
class Element;
class SubstanceData_;
class ReactionData_;
class ReactionSetData_;
class ThermoSetData;
struct ElementKey;
class TraversalData;

using List_VertexId_VertexType    = std::vector< std::pair<std::string, std::string> >;

auto setDatabaseConnectionFilePath(const std::string &filePath) -> void;

class DatabaseClient
{
  public:
    ///
    /// \brief DBClient creates a DBClient instance
    /// \param settingsFile path to the ThermoFun.ini file
    ///
    explicit DatabaseClient( const std::shared_ptr<jsonio::TDataBase>& otherdb );

    DatabaseClient();

    /// Assign a DatabaseClient instance to this instance
    auto operator=(DatabaseClient other) -> DatabaseClient &;

    /// Destroy this instance
    virtual ~DatabaseClient();

    ///
    /// \brief getDatabase reads from the EJDB database substances and reactions with the same sourceTDB
    /// \param sourceTDB data set index
    /// \return returns a ThermoFun Database structure containing the substances and reactions maps
    ///
    auto thermoFunDatabase(unsigned int sourcetdbIndex) -> Database;

    /**
     * @brief parseSubstanceFormula parses a substance formula in GEMS format
     * @param formula in GEMS format
     * @return map of element object and coefficient
     */
    auto parseSubstanceFormula(std::string formula) -> std::map<Element, double>;

    /**
     * @brief getSourcetdbIndexes returns the indexes of all available thermodynamic data sets
     * @return a set of indexes
     */
    auto sourcetdbIndexes() -> std::set<unsigned int>;

    /**
     * @brief sourcetdbNamesIndexes returns a map with all available thermodynamic data sets names and their indexes
     * @param sourcetdbIndexes set of thermodynamic data sets indexes
     * @return map of thermodynamic data sets names and indexes
     */
    auto sourcetdbNamesIndexes(const std::set<unsigned int> &sourcetdbIndexes) -> std::map<std::string, unsigned int>;

    /**
     * @brief sourcetdbNamesComments returns a map with all available thermodynamic data sets names and associated comments
     *          from the json schema
     * @param sourcetdbIndexes set of thermodynamic data sets indexes
     * @return map of thermodynamic data sets names and comments
     */
    auto sourcetdbNamesComments(const std::set<unsigned int> &sourcetdbIndexes) -> std::map<std::string, std::string>;

    /**
     * @brief sourcetdbListAll returns the indexes and name of all available thermodynamic data sets
     * @return a list of  <index>-<name>-<comment>
     */
    auto sourcetdbListAll() -> std::vector<std::string>;

    /**
     * @brief sourcetdbNamesIndexes returns a map with all available thermodynamic data sets names and their indexes
     * @return  map of thermodynamic data sets names and indexes
     */
    auto sourcetdbNamesIndexes() -> std::map<std::string, unsigned int>
    {
        return sourcetdbNamesIndexes(sourcetdbIndexes());
    }

    /**
     * @brief availableElements returns the set of available element symbols, for a given thermodynamic data set index
     * @param sourcetdb thermodynamic data set index
     * @return set of element symbols
     */
    auto availableElements(unsigned int sourcetdb) -> std::set<std::string>;

    /**
     * @brief availableElementsKey returns a vector of all available elements (ElementKey object), for a given thermodynamic data set index
     * @param sourcetdb thermodynamic data set index
     * @return vector of ElementKey objects
     */
    auto availableElementsKey(unsigned int sourcetdb) -> std::vector<ElementKey>;

    /**
     * @brief availableElementsSet returns a set of all available elements (Element object), for a given thermodynamic data set index
     * @param sourcetdb thermodynamic data set index
     * @return set of Element objects
     */
    auto availableElementsSet(int sourcetdb) -> std::set<Element>;


    /**
     * @brief elementIds returns a vector of element ids from ElementKeys
     * @param elements vector of ElementKeys
     * @return vector of Element ids
     */
    auto elementIds( const std::vector<ElementKey>& elements) -> std::vector<std::string>;

    /**
     * @brief availableSubstances returns the list of available substance symbols
     * @param sourcetdb thermodynamic data set index
     * @return vector of substance symbols
     */
    auto availableSubstances(unsigned int sourcetdb) -> std::vector<std::string>;

    /**
     * @brief availableReactions returns the list of available reaction symbols
     * @param sourcetdb thermodynamic data set index
     * @return vector of reaction symbols
     */
    auto availableReactions(unsigned int sourcetdb) -> std::vector<std::string>;

    /**
     * @brief substData returns a SubstanceData object for reading substance data from the database
     * @return SubstanceData object
     */
    auto substData() const -> SubstanceData_&;

    /**
     * @brief reactData returns a ReactionData object for reading reaction data from the database
     * @return ReactionData object
     */
    auto reactData() const -> ReactionData_&;

    /**
     * @brief reactSetData returns a ReactionSetData_ object
     * @return
     */
    auto reactSetData() const -> ReactionSetData_&;

    /**
     * @brief thermoDataSet returns a ThermoDataSet object
     * @return
     */
    auto thermoDataSet() const -> ThermoSetData&;

    /**
     * @brief getTraversal returns a TraversalData object
     * @return
     */
    auto getTraversal() const -> TraversalData&;

    /// Output record and all incoming to json file
    auto BackupAllIncoming( const std::vector<std::string>& ids, const std::string fileName ) -> void;

    /// Collect record and all incoming _ids
    auto TraverseAllIncomingEdges( const std::string& id ) -> List_VertexId_VertexType;

    auto recordsFromThermoDataSet(const std::string& ThermoDataSetSymbol ) -> List_VertexId_VertexType;

private:

    auto extractFieldValuesFromQueryResult(std::vector<std::string> resultQuery, std::string fieldName) -> std::vector<std::string>;

    struct Impl;

    std::shared_ptr<Impl> pimpl;
};
}

#endif // DATABASECLIENT_H
