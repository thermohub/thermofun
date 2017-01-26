#ifdef DBCLIENT_H

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

// Constructor
DBClient::DBClient (string settingsFile)
{
    settings.QtSettings = new QSettings(settingsFile.c_str(), QSettings::IniFormat);
    getDataFromPreferencesFile( );
}

// Desctructor
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
}

//---------------------------------------------------------------------
// Read all thrift schemas from dirPath
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

TDBGraph *DBClient::newDBClinet(string schemaName, string query)
{
    // no schema
    if( schemaName.empty() )
        return nullptr;
    string collcName = settings.collName.toStdString();
    TDBGraph* newClient =TDBGraph::newDBClient( &schema,
                                                dbDrive(settings.useLocalDB), collcName.c_str(), schemaName, query  );
    return newClient;
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
    string qrJson, key, valDB, _idReac;
    bson record;
    Database db;
    vector<string> aKeyList;
    vector<vector<string>> aValList;

    // The set of all aqueous species in the database
    SubstancesMap substances_map;
    // The set of all gaseous species in the database
    ReactionsMap reactions_map;

    qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    substanceVertex = unique_ptr<TDBGraph> (newDBClinet("VertexSubstance", qrJson));

    qrJson = "{ \"_label\" : \"reaction\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    reactionVertex = unique_ptr<bsonio::TDBGraph>(newDBClinet("VertexReaction", qrJson));

    qrJson = "{ \"_label\" : \"takes\"}";
    takesEdge = unique_ptr<TDBGraph> (newDBClinet("EdgeTakes", qrJson));

    qrJson = "{ \"_label\" : \"defines\"}";
    definesEdge = unique_ptr<TDBGraph> (newDBClinet("EdgeDefines", qrJson));

    // get substances
    substanceVertex->GetKeyValueList( aKeyList, aValList );
    for( uint ii=0; ii<aKeyList.size(); ii++ )
    {
        key = aKeyList[ii];
        substanceVertex->GetRecord( key.c_str() );
        valDB = substanceVertex->GetJson();
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
    reactionVertex->GetKeyValueList( aKeyList, aValList );
    for( uint ii=0; ii<aKeyList.size(); ii++ )
    {
        key = aKeyList[ii];
        reactionVertex->GetRecord( key.c_str() );
        valDB = reactionVertex->GetJson();
        jsonToBson( &record, valDB );
        bsonio::bson_to_key( record.data, "_id", _idReac );

        Reaction reaction = parseReaction(record.data);

        if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
             reactions_map[reaction.symbol()] = reaction;
        } else {
          // ERROR reaction with the same symbol found!
        }

        // get reactants by following reaction incoming takes edge
        setReactantsFollowingIncomingTakesEdges(_idReac, reaction);

        // set defined substance
        substances_map[getDefinedSubstanceSymbol(_idReac)].setReactionSymbol(reaction.symbol());
        substances_map[getDefinedSubstanceSymbol(_idReac)].setThermoCalculationType(SubstanceThermoCalculationType::type::REACDC);

        reactions_map[reaction.symbol()] = reaction;
    }

    db.addMapSubstances(substances_map);
    db.addMapReactions(reactions_map);

    return db;
}

void DBClient::setReactantsFollowingIncomingTakesEdges(std::string _id, Reaction &reaction)
{
    std::map<std::string, int> map;
    string kbuf;
    int stoi_coeff;
    bson record;
    string qrJson = "{'_type': 'edge', '_inV': '";
    qrJson += _id;
    qrJson += "' }";

    vector<string> _queryFields = { "_outV", "properties.stoi_coeff"};
    vector<string> _resultDataEdge, _resultDataSubst;
    takesEdge->runQuery( qrJson,  _queryFields, _resultDataEdge );

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "properties.stoi_coeff", kbuf );
        stoi_coeff = atoi(kbuf.c_str());

        bsonio::bson_to_key( record.data, "_outV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        substanceVertex->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataSubst);

        if (_resultDataSubst.size()>0)
        {
            jsonToBson(&record, _resultDataSubst[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
        }

        map.insert(std::pair<std::string,int>(kbuf,stoi_coeff));
    }

    reaction.setReactants(map);
}

std::string DBClient::getDefinedSubstanceSymbol(std::string _id)
{
    string kbuf;
    bson record;
    string qrJson = "{'_type': 'edge', '_outV': '";
    qrJson += _id;
    qrJson += "' }";

    vector<string> _queryFields = { "_inV", "_label"};
    vector<string> _resultDataEdge, _resultDataSubst;
    definesEdge->runQuery( qrJson,  _queryFields, _resultDataEdge );

    if (_resultDataEdge.size()>0)
    {
        jsonToBson(&record, _resultDataEdge[0]);
        bsonio::bson_to_key( record.data, "_inV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        substanceVertex->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataSubst);

        if (_resultDataSubst.size()>0)
        {
            jsonToBson(&record, _resultDataSubst[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
        }
    }

    return kbuf;
}

}

#endif // DBCLIENT_H
