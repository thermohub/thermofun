#ifndef DATABASECLIENT_H
#define DATABASECLIENT_H

#include <memory>
#include <set>

#include <string>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QDir>

#include "formuladata.h"

namespace ThermoFun
{

struct Database;
//struct Reaction;
//struct Substance;
struct Element;
struct SubstanceData;
struct ReactionData;
//struct ElementKey;

//using GetJsonRecord           = std::function<string(string)>;

class DatabaseClient
{
  public:
    ///
    /// \brief DBClient creates a DBClient instance
    /// \param settingsFile path to the ThermoFun.ini file
    ///
    explicit DatabaseClient(std::string settingsFile);

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
    auto thermoFunDatabase(uint sourcetdbIndex) -> Database;

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
    auto sourcetdbIndexes() -> std::set<uint>;

    /**
     * @brief sourcetdbNamesIndexes returns a map with all available thermodynamic data sets names and their indexes
     * @param sourcetdbIndexes set of thermodynamic data sets indexes
     * @return map of thermodynamic data sets names and indexes
     */
    auto sourcetdbNamesIndexes(const std::set<uint> &sourcetdbIndexes) -> std::map<std::string, uint>;

    /**
     * @brief sourcetdbNamesComments returns a map with all available thermodynamic data sets names and associated comments
     *          from the json schema
     * @param sourcetdbIndexes set of thermodynamic data sets indexes
     * @return map of thermodynamic data sets names and comments
     */
    auto sourcetdbNamesComments(const std::set<uint> &sourcetdbIndexes) -> std::map<std::string, std::string>;

    /**
     * @brief sourcetdbNamesIndexes returns a map with all available thermodynamic data sets names and their indexes
     * @return  map of thermodynamic data sets names and indexes
     */
    auto sourcetdbNamesIndexes() -> std::map<std::string, uint>
    {
        return sourcetdbNamesIndexes(sourcetdbIndexes());
    }

    /**
     * @brief availableElements returns the set of available element symbols, for a given thermodynamic data set index
     * @param sourcetdb thermodynamic data set index
     * @return set of element symbols
     */
    auto availableElements(uint sourcetdb) -> std::set<std::string>;

    /**
     * @brief availableElementsKey returns a vector of all available elements (ElementKey object), for a given thermodynamic data set index
     * @param sourcetdb thermodynamic data set index
     * @return vector of ElementKey objects
     */
    auto availableElementsKey(uint sourcetdb) -> std::vector<ElementKey>;

    /**
     * @brief availableElementsSet returns a set of all available elements (Element object), for a given thermodynamic data set index
     * @param sourcetdb thermodynamic data set index
     * @return set of Element objects
     */
    auto availableElementsSet(int sourcetdb) -> std::set<Element>;

    /**
     * @brief availableSubstances returns the list of available substance symbols
     * @param sourcetdb thermodynamic data set index
     * @return vector of substance symbols
     */
    auto availableSubstances(uint sourcetdb) -> std::vector<std::string>;

    /**
     * @brief availableReactions returns the list of available reaction symbols
     * @param sourcetdb thermodynamic data set index
     * @return vector of reaction symbols
     */
    auto availableReactions(uint sourcetdb) -> std::vector<std::string>;

    /**
     * @brief substData returns a SubstanceData object for reading substance data from the database
     * @return SubstanceData object
     */
    auto substData() const -> SubstanceData;

    /**
     * @brief reactData returns a ReactionData object for reading reaction data from the database
     * @return ReactionData object
     */
    auto reactData() const -> ReactionData;

  private:
    auto recordsFieldValues(std::vector<std::string> resultQuery, std::string fieldName) -> std::vector<std::string>;

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

#endif // DATABASECLIENT_H
