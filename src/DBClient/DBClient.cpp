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

    dbgraph = unique_ptr<TDBGraph> (newDBClinet(vertName, vertQuery));
    auto qrJson = "{ \"_label\" : \"element\"}";
    elementVertex = unique_ptr<bsonio::TDBGraph> (newDBClinet( "VertexElement", qrJson ));
    // load all elements into system
    ChemicalFormula::setDBElements( elementVertex.get(), qrJson );
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
    set<ElementKey> elements;
    mapFormulaElements map;
    FormulaToken formula("");

    formula.setFormula(  formula_ );
    elements.insert( formula.getElements().begin(), formula.getElements().end());

    for (auto element : elements)
    {
        Element e;
        auto itrdb = ChemicalFormula::getDBElements().find(element);
        if( itrdb ==  ChemicalFormula::getDBElements().end() )
            bsonio::bsonioErr( "E37FPrun: Invalid symbol ", element.symbol );

        e.setClass(element.class_);
        e.setIsotopeMass(element.isotope);
        e.setSymbol(element.symbol);
        e.setName(itrdb->second.name);
        e.setMolarMass(itrdb->second.atomic_mass);
        e.setEntropy(itrdb->second.entropy);
        e.setHeatCapacity(itrdb->second.heat_capacity);
        e.setVolume(itrdb->second.volume);
        e.setValence(itrdb->second.valence);

        map[e]++;
    }

    return map;
}

set<int> DBClient::getSourcetdbNums()
{
  string query = "{ \"$or\" : [ { \"_label\" :   \"substance\" },"
                               "{ \"_label\" :   \"reaction\"  }"
                              "], \"_type\" :   \"vertex\"  }  ";
  set<int> _sourcetdb;
  vector<string> _resultData = dbgraph->fieldQuery("properties.sourcetdb", query );

  for( uint ii=0; ii<_resultData.size(); ii++)
  {
      int asourcetdb =  stoi( _resultData[ii] );
     _sourcetdb.insert(asourcetdb);
  }
  return _sourcetdb;
}

vector<string> DBClient::getSourcetdbNames( const set<int>& sourcetdb )
{
  // set lists
  vector<string> _sourcetdbList;
  bsonio::ThriftEnumDef* enumdef =  schema.getEnum("SourceTDB" );
  if(enumdef != nullptr )
  {
      foreach( int idx, sourcetdb)
      {
         string name = enumdef->getNamebyId(idx);
         name += " - " + enumdef->getDoc(name);
         name = to_string(idx) + "-" + name;
         _sourcetdbList.push_back(name);
      }
  }
  return _sourcetdbList;
}

mapFormulaElements DBClient::makeAvailableElementsList( int sourcendx )
{
   set<ElementKey> elements;
   mapFormulaElements map;

   string query = "{ \"_label\" :   \"substance\", "
                    " \"_type\" :   \"vertex\", "
                    " \"properties.sourcetdb\" :   ";
           query +=  to_string(sourcendx);
           query += " }  ";
   vector<string> _queryFields = { "_id",
                                    "properties.formula",
                                    "properties.symbol",
                                  };
    vector<string> _resultData;
    dbgraph->runQuery( query,  _queryFields, _resultData );

    FormulaToken parser("");
    foreach( string subitem, _resultData )
    {
      string formula =  bsonio::extractStringField( "formula", subitem );
      string symbol =  bsonio::extractStringField( "symbol", subitem );
      //  cout << subitem << "      " << formula << "  " << symbol << endl;
      // test elements
      parser.exeptionCheckElements( symbol, formula );
      parser.setFormula(formula);
      elements.insert( parser.getElements().begin(), parser.getElements().end());
    }

    for (auto element : elements)
    {
        Element e;
        auto itrdb = ChemicalFormula::getDBElements().find(element);
        if( itrdb ==  ChemicalFormula::getDBElements().end() )
            bsonio::bsonioErr( "E37FPrun: Invalid symbol ", element.symbol );

        e.setClass(element.class_);
        e.setIsotopeMass(element.isotope);
        e.setSymbol(element.symbol);
        e.setName(itrdb->second.name);
        e.setMolarMass(itrdb->second.atomic_mass);
        e.setEntropy(itrdb->second.entropy);
        e.setHeatCapacity(itrdb->second.heat_capacity);
        e.setVolume(itrdb->second.volume);
        e.setValence(itrdb->second.valence);

        map[e]++;
    }

    return map;
}

auto DBClient::getMapSubstances(const mapFormulaElements& elements, vector<string> substKeyList, vector<vector<string>> substValList) -> void
{

}

auto DBClient::getMapReactions(const mapFormulaElements& elements, vector<string> reactKeyList) -> void
{
    for (auto id : reactKeyList)
    {

    }
}


auto DBClient::getDatabase( int sourcetdb, const mapFormulaElements& elements ) -> ThermoFun::Database
{
    // get substance keys containing the selected elements

    vector<string> substKeyList, reactKeyList;
    vector<vector<string>> substValList, reactValList;

    MapIdType mapSubst, mapReact;

    auto qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourcetdb)+ "}]}";
    substanceVertex = unique_ptr<TDBGraph> (newDBClinet("VertexSubstance", qrJson));

    qrJson = "{ \"_label\" : \"reaction\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourcetdb)+ "}]}";
    reactionVertex = unique_ptr<bsonio::TDBGraph>(newDBClinet("VertexReaction", qrJson));

    substanceVertex->GetKeyValueList( substKeyList, substValList );
    reactionVertex->GetKeyValueList( reactKeyList, reactValList );

//    mapSubst = getMapSubstances (elements, substKeyList, substValList);
//    mapReact = getMapReactions (elements, reactKeyList, reactValList);

    MapIdType mapAll = mapSubst; mapAll.insert(mapReact.begin(), mapReact.end());

    Traversal tr(dbgraph);

    return tr.getDatabaseFromMapOfIds(mapAll, "0");
}


}

