#ifdef DBCLIENT_H

#include <string>
#include <QFileInfo>
#include <QSettings>

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"

namespace ThermoFun {

struct Database;
struct Substance;
struct Reaction;

///
/// \brief The DBSettings struct holds the settings for connecting to the local or server database
///
struct DBSettings
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

class DBClient
{
    DBSettings settings;

    ThriftSchema schema;

    // keys list data
    boost::shared_ptr<bsonio::TDBGraph> dbgraph;

    // Connect to DataBase
    unique_ptr<bsonio::TDBGraph> reactionVertex;
    unique_ptr<bsonio::TDBGraph> substanceVertex;
    unique_ptr<bsonio::TDBGraph> takesEdge;
    unique_ptr<bsonio::TDBGraph> definesEdge;

    std::map<std::string, bson> map_id_bson;

    // reading shcemas folder
    void readSchemaDir( const QString& dirPath );
    // reads settings from preferences ThermoFun.ini file
    void getDataFromPreferencesFile( );
    // resets DBClient with schema and query
    void resetDBClinet(string curSchemaName, string query);
    // returns new DBClient using schema and query
    bsonio::TDBGraph *newDBClinet(string schemaName, string query);
    // sets the reactans following the incoming takes edges of reaction with database _id
    void setReactantsFollowingIncomingTakesEdges(std::string _id, Reaction &reaction);
    // sets the symbol of the reaction which defines the substance by folowing the incoming defines edge
    std::string getDefinesReactionSymbol(std::string _idSubst);
    // returns the substance symbol which is defined by the reaction with the database _id
    std::string getDefinedSubstanceSymbol(std::string _id);

public:
    ///
    /// \brief DBClient creates a DBClient instance
    /// \param settingsFile path to the ThermoFun.ini file
    ///
    explicit DBClient(std::string settingsFile);
    ~DBClient();

    ///
    /// \brief getDatabase reads from the EJDB database substances and reactions with the same sourceTDB
    /// \param sourceTDB
    /// \return returns a ThermoFun Database structure containing the substances and reactions maps
    ///
    auto getDatabase(uint sourceTDB) -> Database;

};


}

#endif // DBCLIENT_H
