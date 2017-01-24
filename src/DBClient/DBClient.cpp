#include "DBClient.h"
#include "bsonio/dbdriverejdb.h"
#include "bsonio/dbclient.h"

#include "Database.h"
#include "ReadFiles.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

using namespace bsonio;

namespace ThermoFun {

DBClient::DBClient (string settingsFile)
{

//    settings = new DBSettings;
    settings.QtSettings = new QSettings(settingsFile.c_str(), QSettings::IniFormat);
    getDataFromPreferencesFile( );
}

DBClient::~DBClient()
{
    delete settings.QtSettings;
}

void DBClient::getDataFromPreferencesFile()
{
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

//    // Connect to DataBase
//    unique_ptr<TDBGraph> reactionVertex( newDBGraphClient( "VertexReaction", "" ) );
//    unique_ptr<TDBGraph> substanceVertex( newDBGraphClient( "VertexSubstance", "" ) );
//    unique_ptr<TDBGraph> takeEdge( newDBGraphClient( "EdgeTakes", "" ) );
//    unique_ptr<TDBGraph> defineEdge( newDBGraphClient( "EdgeDefines", "" ) );

}

//---------------------------------------------------------------------
// Read all thrift schemas from Dir
void DBClient::readSchemaDir( const QString& dirPath )
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

void DBClient::resetDBClinet(string curSchemaName, string query)
{
    string coll = settings.collName.toStdString();
//    if( isDefaultQuery)
//        defaultQuery = _data.query;
    TDBGraph* newClient = TDBGraph::newDBClient( &schema,
                                                 dbDrive(settings.useLocalDB), coll.c_str(), curSchemaName, query  ); /*newDBClient( curSchemaName, defaultQuery );*/
    // no schema
    if( newClient == nullptr )
    {
        dbgraph.reset();
        return;
    }
    dbgraph.reset( newClient );
    try{
        dbgraph->Open();
    }
    catch(bsonio_exeption& e)
    {
//        ERROR std::cout << "Internal comment " << e.title() << e.what() << endl;
    }

    dbgraph->setKey( "*" );

}


auto DBClient::getDatabase(uint sourceTDB) -> Database
{
    string qrJson;
    bson record;
    Database db;
    vector<string> aKeyList;
    vector<vector<string>> aValList;

    /// The set of all aqueous species in the database
    SubstancesMap substances_map;
    /// The set of all gaseous species in the database
    ReactionsMap reactions_map;

    _shemaNames.push_back("VertexSubstance");
    _shemaNames.push_back("VertexReaction");
    _shemaNames.push_back("VertexReactionSet");

    // get substances
    qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";

    resetDBClinet(_shemaNames[0], qrJson);
    dbgraph->GetKeyValueList( aKeyList, aValList );
    for( uint ii=0; ii<aKeyList.size(); ii++ )
    {
        string key = aKeyList[ii];
        dbgraph->GetRecord( key.c_str() );
        string valDB = dbgraph->GetJson();
        jsonToBson( &record, valDB );

        Substance substance = parseSubstance(record.data);

        if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
             substances_map[substance.symbol()] = substance;
        } else {
          // ERROR substance with the same symbol found!
        }

        substances_map[substance.symbol()] = substance;
    }

    // get reactions
    qrJson = "{ \"_label\" : \"reaction\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";

    resetDBClinet(_shemaNames[0], qrJson);
    dbgraph->GetKeyValueList( aKeyList, aValList );
    for( uint ii=0; ii<aKeyList.size(); ii++ )
    {
        string key = aKeyList[ii];
        dbgraph->GetRecord( key.c_str() );
        string valDB = dbgraph->GetJson();
        jsonToBson( &record, valDB );

//        Reaction reaction = parseReaction(record.data);

//        if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
//             reactions_map[reaction.symbol()] = reaction;
//        } else {
//          // ERROR reaction with the same symbol found!
//        }

//        reactionss_map[reaction.symbol()] = reaction;
    }

    // work with the edges

    db.addMapSubstances(substances_map);
    db.addMapReactions(reactions_map);

    return db;
}


}
