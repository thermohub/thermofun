#ifndef DBCLIENT_H
#define DBCLIENT_H

#include <string>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

#include <map>
#include <QtCore/QDir>


// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"

namespace ThermoFun {

struct Database;
struct Reaction;
struct Substance;
struct Element;
//struct ElementKey;

using mapFormulaElements = std::map<Element, double>;

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

///
/// \brief The DBClient class is used for comunicating with the database and retrieving data using queries
///
class DBClient
{
    /// Vertex name
    const string vertName = "VertexSubstance";
    /// Default query to vertex
    const string vertQuery = "{ \"_label\" :   \"substance\" }";
    /// Fields to be extracted
    vector<string> vertFields;
    /// Columns names ( for extracted data )
    vector<string> vertHeads;

    DBSettings settings;

    bsonio::ThriftSchema schema;

    // keys list data
    boost::shared_ptr<bsonio::TDBGraph> dbgraph;

    // Connect to DataBase
    unique_ptr<bsonio::TDBGraph> reactionVertex;
    unique_ptr<bsonio::TDBGraph> substanceVertex;
    unique_ptr<bsonio::TDBGraph> takesEdge;
    unique_ptr<bsonio::TDBGraph> definesEdge; 
    unique_ptr<bsonio::TDBGraph> elementVertex;

    std::map<std::string, bson> map_id_bson;

    // reading shcemas folder
    void readSchemaDir( const QString& dirPath );
    // reads settings from preferences ThermoFun.ini file
    void getDataFromPreferencesFile( );
    // resets DBClient with schema and query
    void resetDBClinet(string curSchemaName, string query);
    // returns new DBClient using schema and query
    bsonio::TDBGraph *newDBClinet(string schemaName, string query);

    auto getMapReactions(const mapFormulaElements& elements, vector<string> reactKeyList) -> void;
    auto getMapSubstances(const mapFormulaElements& elements, vector<string> substKeyList, vector<vector<string>> substValList) -> void;

public:
    ///
    /// \brief DBClient creates a DBClient instance
    /// \param settingsFile path to the ThermoFun.ini file
    ///
    explicit DBClient(std::string settingsFile);

//    ThermoDataAbstract( const string& name, const string& query,
//                        const vector<string>& fields, const vector<string>& headers ):
//        vertName(name), vertQuery(query),
//        vertFields(fields), vertHeads(headers)

    DBClient();

    /// Construct a copy of an DBClient instance
    DBClient(const DBClient& other);

    /// Assign a DBClient instance to this instance
    auto operator=(DBClient other) -> DBClient&;

    virtual ~DBClient();

    ///
    /// \brief getDatabase reads from the EJDB database substances and reactions with the same sourceTDB
    /// \param sourceTDB
    /// \return returns a ThermoFun Database structure containing the substances and reactions maps
    ///
    auto getDatabase(uint sourceTDB) -> Database;

    auto getDatabase( int sourcetdb, const mapFormulaElements& elements ) -> ThermoFun::Database;

    auto parseSubstanceFormula (std::string formula) -> mapFormulaElements;


    // Special functions
    /// Get Sourcetdb numbers present into data base
    set<int> getSourcetdbNums();
    /// Build Sourcetdb Names list from indexes
    vector<string> getSourcetdbNames( const set<int>& sourcetdb);
    /// Get Sourcetdb Names present into data base
    vector<string> getSourcetdbList()
    {
      return getSourcetdbNames( getSourcetdbNums() );
    }

    mapFormulaElements makeAvailableElementsList( int sourcendx );

};


}

#endif // DBCLIENT_H
