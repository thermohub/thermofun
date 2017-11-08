#ifndef DATABASECLIENT_H
#define DATABASECLIENT_H

#include <memory>
#include <set>


#include <string>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QDir>

#include "formuladata.h"

namespace ThermoFun {

struct Database;
//struct Reaction;
//struct Substance;
struct Element;
struct SubstanceData;
struct ReactionData;
//struct ElementKey;

///
/// \brief The DBSettings struct holds the settings for connecting to the local or server database
///
struct DatabaseClientSettings
{
    /// Object for reading ini settings file
    QSettings *QtSettings;
    /// Schemas folder
    QString schemaDir;
    /// Local folder with the EJDB database
    QString localDBDir;
    /// Name of the local EJDB database
    QString localDBName;
    /// Path to the EJDB database file
    QFileInfo file;
    /// name of database collection
    QString collName;
    /// true if we use local database
    bool useLocalDB;
};

class DatabaseClient
{
public:

    ///
    /// \brief DBClient creates a DBClient instance
    /// \param settingsFile path to the ThermoFun.ini file
    ///
    explicit DatabaseClient(std::string settingsFile);

    DatabaseClient( );

    /// Assign a DatabaseClient instance to this instance
    auto operator=(DatabaseClient other) -> DatabaseClient&;

    /// Destroy this instance
    virtual ~DatabaseClient();

    ///
    /// \brief getDatabase reads from the EJDB database substances and reactions with the same sourceTDB
    /// \param sourceTDB
    /// \return returns a ThermoFun Database structure containing the substances and reactions maps
    ///
    auto getDatabase( uint sourcetdbIndex) -> Database;

    auto getDatabase( int sourcetdbIndex, const std::map<Element, double>& elements ) -> Database;

    auto parseSubstanceFormula (std::string formula) -> std::map<Element, double>;

    // Special functions
    /// Get Sourcetdb numbers present into data base
    std::set<int> getSourcetdbIndexes();

    /// Build Sourcetdb Names set from indexes
    std::map<std::string, uint> sourcetdbNamesIndexes(const std::set<int>& sourcetdbIndexes);

    /// Build Sourcetdb Names set from indexes
    std::map<std::string, std::string> sourcetdbNamesComments(const std::set<int>& sourcetdbIndexes);

    /// Get Sourcetdb Names present into data base
    std::map<std::string, uint> getSourcetdbList()
    {
      return sourcetdbNamesIndexes( getSourcetdbIndexes() );
    }

    std::set<std::string> availableElements(int sourcetdb );
    std::vector<ElementKey> availableElementsList_( int sourcetdb );

    SubstanceData substData() const;
    ReactionData reactData() const;

private:

    std::set<Element> availableElementsList(int sourcetdb );

    struct Impl;

    std::shared_ptr<Impl> pimpl;
};




}

#endif // DATABASECLIENT_H
