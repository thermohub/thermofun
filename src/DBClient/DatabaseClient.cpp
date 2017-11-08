#include "DatabaseClient.h"

// bonio includes
#include "bsonio/thrift_schema.h"
#include "bsonio/dbgraph.h"
#include "bsonio/dbdriverejdb.h"
#include "bsonio/dbclient.h"

#include "SubstanceData.h"
#include "ReactionData.h"

#include "../Database.h"
#include "../Element.h"
#include "Traversal.h"

using namespace bsonio;

namespace ThermoFun {


struct DatabaseClient::Impl
{
    /// settings for database connection
    DatabaseClientSettings settings;

    /// schema for parsing the data
    ThriftSchema schema;

    /// access to substance records
    SubstanceData substData;

    /// access to reaction records
    ReactionData reactData;

    Impl(std::string settingsFile)
    {
        getDataFromSettingsFile(settingsFile);
    }

    Impl()
    {}

    TDBGraph *newDBClinet(string schemaName, string query)
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
        string qrJson;
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

        // default connections to vertexes
        qrJson = "{ \"_label\" : \"substance\" }";
        substData.setDB(boost::shared_ptr<bsonio::TDBGraph> (newDBClinet("VertexSubstance", qrJson)));
        qrJson = "{ \"_label\" : \"reaction\" }";
        reactData.setDB(boost::shared_ptr<bsonio::TDBGraph> (newDBClinet("VertexReaction", qrJson)));
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

//DatabaseClient::DatabaseClient( boost::shared_ptr<bsonio::TDBGraph> dbgraph )
//: pimpl(new Impl( ))
//{ }

DatabaseClient::DatabaseClient( )
: pimpl(new Impl( ))
{ }

//DatabaseClient::DatabaseClient(const DatabaseClient& other)
//: pimpl(new Impl(*other.pimpl))
//{}

auto DatabaseClient::operator=(DatabaseClient other) -> DatabaseClient&
{
    pimpl = std::move(other.pimpl);
    return *this;
}


DatabaseClient::~DatabaseClient()
{ }

auto DatabaseClient::getDatabase(uint sourceTDB) -> Database
{
    string qrJson;
    Database db;
    vector<string> substKeyList, reactKeyList, keyList;
    vector<vector<string>> substValList, reactValList;

    qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    pimpl->substData.setDB(boost::shared_ptr<bsonio::TDBGraph> (pimpl->newDBClinet("VertexSubstance", qrJson)));

    qrJson = "{ \"_label\" : \"reaction\", \"$and\" : [{\"properties.sourcetdb\" : "+to_string(sourceTDB)+ "}]}";
    pimpl->reactData.setDB(boost::shared_ptr<bsonio::TDBGraph>(pimpl->newDBClinet("VertexReaction", qrJson)));

    pimpl->substData.getDB()->GetKeyValueList( substKeyList, substValList );
//    pimpl->reactData.getDB()->GetKeyValueList( reactKeyList, reactValList );

    // get substances
    keyList.insert(keyList.end(), substKeyList.begin(), substKeyList.end());
//    keyList.insert(keyList.end(), reactKeyList.begin(), reactKeyList.end());

    Traversal tr(pimpl->substData.getDB());
    // get all data connected to the substances using level 0 for reaction defined substances
    db = tr.getDatabaseFromMapOfIds(tr.getMapOfConnectedIds(keyList, "0"), "0");
    return db;
}

auto DatabaseClient::parseSubstanceFormula(std::string formula_) -> mapElements
{
    std::set<ElementKey> elements;
    mapElements map;
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

set<int> DatabaseClient::getSourcetdbNums()
{
  string query = "{ \"$or\" : [ { \"_label\" :   \"substance\" },"
                               "{ \"_label\" :   \"reaction\"  }"
                              "], \"_type\" :   \"vertex\"  }  ";
  set<int> _sourcetdb;
  vector<string> _resultData = pimpl->substData.getDB()->fieldQuery("properties.sourcetdb", query );

  for( uint ii=0; ii<_resultData.size(); ii++)
  {
      int asourcetdb =  stoi( _resultData[ii] );
     _sourcetdb.insert(asourcetdb);
  }
  return _sourcetdb;
}

vector<string> DatabaseClient::getSourcetdbNames( const set<int>& sourcetdb )
{
  // set lists
  vector<string> _sourcetdbList;
  bsonio::ThriftEnumDef* enumdef =  pimpl->schema.getEnum("SourceTDB" );
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

set<Element> DatabaseClient::availableElementsList( int sourcetdb )
{
   std::set<ElementKey> elements;
   set<Element> set;

   string query = "{ \"_label\" :   \"substance\", "
                    " \"_type\" :   \"vertex\", "
                    " \"properties.sourcetdb\" :   ";
           query +=  to_string(sourcetdb);
           query += " }  ";
   vector<string> _queryFields = { "_id",
                                    "properties.formula",
                                    "properties.symbol",
                                  };
    vector<string> _resultData;
    pimpl->substData.getDB()->runQuery( query,  _queryFields, _resultData );

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

        set.insert(e);
    }
    return set;
}

vector<string> DatabaseClient::availableElements( int sourcetdb )
{
   vector<string> set;

   auto elements = availableElementsList(sourcetdb);

    for (auto element : elements)
    {
        set.push_back(element.symbol());
    }
    return set;
}


std::vector<ElementKey> DatabaseClient::availableElementsList_( int sourcetdb )
{
   std::set<ElementKey> elements;
   std::vector<ElementKey> set;

   string query = "{ \"_label\" :   \"substance\", "
                    " \"_type\" :   \"vertex\", "
                    " \"properties.sourcetdb\" :   ";
           query +=  to_string(sourcetdb);
           query += " }  ";
   vector<string> _queryFields = { "_id",
                                    "properties.formula",
                                    "properties.symbol",
                                  };
    vector<string> _resultData;
    pimpl->substData.getDB()->runQuery( query,  _queryFields, _resultData );

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
        auto itrdb = ChemicalFormula::getDBElements().find(element);
        if( itrdb ==  ChemicalFormula::getDBElements().end() )
            bsonio::bsonioErr( "E37FPrun: Invalid symbol ", element.symbol );
        set.push_back(element);
    }
    return set;
}

SubstanceData DatabaseClient::substData() const
{
    return pimpl->substData;
}

ReactionData DatabaseClient::reactData() const
{
    return pimpl->reactData;
}



}
