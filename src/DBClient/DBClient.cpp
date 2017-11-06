#include "DBClient.h"
#include "bsonio/dbdriverejdb.h"
#include "bsonio/dbclient.h"

#include "../Database.h"
#include "../Substance.h"
#include "../Reaction.h"
#include "../ReadFiles.h"
#include "../Element.h"
#include "formuladata.h"
#include "Traversal.h"

using namespace bsonio;

namespace ThermoFun {

// Constructor
DBClient::DBClient (string settingsFile)
{
    settings.QtSettings = new QSettings(settingsFile.c_str(), QSettings::IniFormat);
    getDataFromPreferencesFile( );
}

auto DBClient::operator=(DBClient other) -> DBClient&
{
    settings = other.settings;
    getDataFromPreferencesFile( );
    return *this;
}

DBClient::DBClient ()
{}

// Desctructor
DBClient::~DBClient()
{}

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
    string qrJson;
    Database db;
    vector<string> substKeyList, reactKeyList, keyList;
    vector<vector<string>> substValList, reactValList;

    qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    dbgraph = unique_ptr<TDBGraph> (newDBClinet("VertexSubstance", qrJson));

    qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    substanceVertex = unique_ptr<TDBGraph> (newDBClinet("VertexSubstance", qrJson));

    qrJson = "{ \"_label\" : \"reaction\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    reactionVertex = unique_ptr<bsonio::TDBGraph>(newDBClinet("VertexReaction", qrJson));

    qrJson = "{ \"_label\" : \"takes\"}";
    takesEdge = unique_ptr<TDBGraph> (newDBClinet("EdgeTakes", qrJson));

    qrJson = "{ \"_label\" : \"defines\"}";
    definesEdge = unique_ptr<TDBGraph> (newDBClinet("EdgeDefines", qrJson));

    qrJson = "{ \"_label\" : \"element\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    elementVertex = unique_ptr<bsonio::TDBGraph> (newDBClinet( "VertexElement", qrJson ));
    // load all elements into system
    ChemicalFormula::setDBElements( elementVertex.get(), qrJson );

    // get substances
    substanceVertex->GetKeyValueList( substKeyList, substValList );
    reactionVertex->GetKeyValueList( reactKeyList, reactValList );
    keyList.insert(keyList.end(), substKeyList.begin(), substKeyList.end());
    keyList.insert(keyList.end(), reactKeyList.begin(), reactKeyList.end());

    Traversal tr(dbgraph);

    db = tr.getDatabaseFromMapOfIds(tr.getMapOfConnectedIds(keyList, "0"), "0");

    return db;
}

auto DBClient::parseSubstanceFormula(std::string formula_) -> mapFormulaElements
{
    map<ElementKey, double> elements;
    mapFormulaElements map;
    FormulaToken formula("");

    formula.setFormula(  formula_ );
    elements.insert( formula.getElements().begin(), formula.getElements().end());

    for (auto element : elements)
    {
        Element e;
        auto itrdb = ChemicalFormula::getDBElements().find(element.first);
        if( itrdb ==  ChemicalFormula::getDBElements().end() )
            bsonio::bsonioErr( "E37FPrun: Invalid symbol ", element.first.symbol );

        e.setClass(element.first.class_);
        e.setIsotopeMass(element.first.isotope);
        e.setSymbol(element.first.symbol);
        e.setName(itrdb->second.name);
        e.setMolarMass(itrdb->second.atomic_mass);
        e.setEntropy(itrdb->second.entropy);
        e.setHeatCapacity(itrdb->second.heat_capacity);
        e.setVolume(itrdb->second.volume);
        e.setValence(itrdb->second.valence);

        map[e] = element.second;
    }

    return map;
}

}

