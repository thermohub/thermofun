#include "DatabaseClient.h"

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"
#include "bsonio/dbdriverejdb.h"
#include "bsonio/dbclient.h"

#include "SubstanceData.h"

#include "../Database.h"
#include "Traversal.h"

using namespace bsonio;

namespace ThermoFun {


struct DatabaseClient::Impl
{
    DatabaseClientSettings settings;
    bsonio::ThriftSchema schema;

    SubstanceData substData;

    Impl(std::string settingsFile)
    {
        getDataFromSettingsFile(settingsFile);
    }

    Impl()
    {}

    bsonio::TDBGraph *newDBClinet(string schemaName, string query)
    {
        // no schema
        if( schemaName.empty() )
            return nullptr;
        string collcName = settings.collName.toStdString();
        TDBGraph* newClient =TDBGraph::newDBClient( &schema,
                                                    dbDrive(settings.useLocalDB), collcName.c_str(), schemaName, query  );
        return newClient;
    }

    void getDataFromSettingsFile(std::string settingsFile)
    {
        settings.QtSettings = new QSettings(settingsFile.c_str(), QSettings::IniFormat);

        if( !settings.QtSettings)
            return;

        settings.schemaDir      = settings.QtSettings->value("SchemasDirectory", "./Resources/data/schemas").toString();
        settings.localDBDir     = settings.QtSettings->value("LocalDBDirectory", "./Resources/data/EJDB").toString();
        settings.localDBName    = settings.QtSettings->value("LocalDBName", "localdb").toString();
        settings.useLocalDB     = settings.QtSettings->value("UseLocalDB", true).toBool();
        settings.file           = QFileInfo(settings.localDBDir, settings.localDBName);
        settings.collName       = settings.QtSettings->value("LocalDBCollection", "data").toString().toUtf8().data();

        TEJDBDriveOne::flEJPath = settings.file.filePath().toUtf8().data();

        // server db defaults
        // The host that the socket is connected to
        TDBClientOne::theHost   =  settings.QtSettings->value("DBSocketHost",
                                                              TDBClientOne::theHost.c_str()).toString().toUtf8().data();
        // The port that the socket is connected to
        TDBClientOne::thePort   = settings.QtSettings->value("DBSocketPort",
                                                             TDBClientOne::thePort ).toInt();

        readSchemaDir( settings.schemaDir );
        SchemaNode::_schema =    &schema;
    }

    void readSchemaDir( const QString& dirPath )
    {
        if( dirPath.isEmpty() )
            return;

        QDir dir(dirPath);
        QStringList nameFilter;
        nameFilter << "*.schema.json";
        QFileInfoList files = dir.entryInfoList( nameFilter, QDir::Files );

        schema.clearAll();
        string fName;
        foreach (QFileInfo file, files)
        {
            //   fName = file.absoluteFilePath().toUtf8().data();
            fName = file.filePath().toUtf8().data();
            //cout << "FILE: " << fName << endl;
            schema.addSchemaFile( fName.c_str() );
        }
    }

};

DatabaseClient::DatabaseClient( std::string settingsFile )
: pimpl(new Impl(settingsFile))
{ }

DatabaseClient::DatabaseClient( )
: pimpl(new Impl( ))
{ }

DatabaseClient::~DatabaseClient()
{ }


auto DatabaseClient::getDatabase(uint sourceTDB) -> Database
{
    string qrJson;
    Database db;
    vector<string> substKeyList, reactKeyList, keyList;
    vector<vector<string>> substValList, reactValList;

    qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
     boost::shared_ptr<bsonio::TDBGraph> substanceVertex =  boost::shared_ptr<bsonio::TDBGraph> (pimpl->newDBClinet("VertexSubstance", qrJson));

    qrJson = "{ \"_label\" : \"reaction\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    auto reactionVertex = unique_ptr<bsonio::TDBGraph>(pimpl->newDBClinet("VertexReaction", qrJson));

    // get substances
    substanceVertex->GetKeyValueList( substKeyList, substValList );
    reactionVertex->GetKeyValueList( reactKeyList, reactValList );
    keyList.insert(keyList.end(), substKeyList.begin(), substKeyList.end());
    keyList.insert(keyList.end(), reactKeyList.begin(), reactKeyList.end());

    Traversal tr(substanceVertex);

    db = tr.getDatabaseFromMapOfIds(tr.getMapOfConnectedIds(keyList, "0"), "0");

    return db;
}






}
