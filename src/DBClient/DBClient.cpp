#include "DBClient.h"
#include "../Database.h"
#include "../Substance.h"
#include "../Reaction.h"
#include "../ReadFiles.h"
#include "../Element.h"
#include "formuladata.h"
#include "Traversal.h"
#ifdef addBSONIO
#include "dbconnect.h"
#else
#include "bsonio/dbconnect.h"
#endif

using namespace bsonio;

namespace ThermoFun {


// Constructor
DBClient::DBClient(boost::shared_ptr<bsonio::TDataBase>& adbconnections):
    dbconnections( adbconnections )
{ }

auto DBClient::operator=(DBClient other) -> DBClient&
{
    dbconnections = other.dbconnections;
    return *this;
}

// bsonio::TDataBase() reads all data from settings file
// and make default connections to DataBase
DBClient::DBClient(): dbconnections( new bsonio::TDataBase() )
{}

// Desctructor
DBClient::~DBClient()
{}

auto DBClient::getDatabase(uint sourceTDB) -> Database
{
    string qrJson;
    Database db;
    vector<string> substKeyList, reactKeyList, keyList;
    vector<vector<string>> substValList, reactValList;

    qrJson = "{ \"_label\" : \"substance\", \"properties.sourcetdb\" : "+to_string(sourceTDB)+ " }";
    substanceVertex = unique_ptr<TDBVertexDocument> ( TDBVertexDocument::newDBVertexDocument(
                         dbconnections.get(), "VertexSubstance", qrJson));

    qrJson = "{ \"_label\" : \"reaction\", \"properties.sourcetdb\" : "+to_string(sourceTDB)+ " }";
    reactionVertex = unique_ptr<TDBVertexDocument> ( TDBVertexDocument::newDBVertexDocument(
                         dbconnections.get(), "VertexReaction", qrJson));

    qrJson = "{ \"_label\" : \"takes\"}";
    takesEdge = unique_ptr<TDBEdgeDocument> (TDBEdgeDocument::newDBEdgeDocument(
              dbconnections.get(), "EdgeTakes", qrJson));

    qrJson = "{ \"_label\" : \"defines\"}";
    definesEdge = unique_ptr<TDBEdgeDocument> (TDBEdgeDocument::newDBEdgeDocument(
               dbconnections.get(),"EdgeDefines", qrJson));

    qrJson = "{ \"_label\" : \"element\", \"properties.sourcetdb\" : "+to_string(sourceTDB)+ " }";
    elementVertex = unique_ptr<TDBVertexDocument> (TDBVertexDocument::newDBVertexDocument(
                        dbconnections.get(),  "VertexElement", qrJson ));
    // load all elements into system
    ChemicalFormula::setDBElements( elementVertex.get(), qrJson );

    // get substances
    substanceVertex->GetKeyValueList( substKeyList, substValList );
    reactionVertex->GetKeyValueList( reactKeyList, reactValList );
    keyList.insert(keyList.end(), substKeyList.begin(), substKeyList.end());
    keyList.insert(keyList.end(), reactKeyList.begin(), reactKeyList.end());


    qrJson = "{ \"_label\" : \"substance\", \"properties.sourcetdb\" : "+to_string(sourceTDB)+ " }";
    dbvertex = boost::shared_ptr<TDBVertexDocument> ( TDBVertexDocument::newDBVertexDocument(
                         dbconnections.get(), "VertexSubstance", qrJson));
    dbvertex->resetMode(true); // coult change schema

    qrJson = "{ \"_label\" : \"takes\"}";
    dbedge = boost::shared_ptr<TDBEdgeDocument> (TDBEdgeDocument::newDBEdgeDocument(
              dbconnections.get(), "EdgeTakes", qrJson));
    dbedge->resetMode(true); // coult change schema
    Traversal tr(dbvertex, dbedge);
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

