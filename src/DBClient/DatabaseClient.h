#ifndef DATABASECLIENT_H
#define DATABASECLIENT_H

#include <memory>

#include <string>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QDir>

namespace ThermoFun {

struct Database;
//struct Reaction;
//struct Substance;

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

//    /// Construct a copy of an DatabaseClient instance
//    DatabaseClient(const DatabaseClient& other);

    /// Assign a DatabaseClient instance to this instance
//    auto operator=(DatabaseClient other) -> DatabaseClient&;

    /// Destroy this instance
    virtual ~DatabaseClient();


    ///
    /// \brief getDatabase reads from the EJDB database substances and reactions with the same sourceTDB
    /// \param sourceTDB
    /// \return returns a ThermoFun Database structure containing the substances and reactions maps
    ///
    auto getDatabase(uint sourceTDB) -> Database;


private:

    struct Impl;

    std::shared_ptr<Impl> pimpl;
};




}

#endif // DATABASECLIENT_H
