#include <sys/time.h>
#include <QScrollBar>
#include <QHeaderView>
#include "jsonio/jsondomfree.h"
#include "ThermoFunWidgetPrivate.h"
#include "ui_ThermoFunWidget.h"
#include "containers.h"

// ThermoFun includes
#ifdef FROM_SRC
#include "../src/Batch/ThermoBatch.h"
#include "../src/Database.h"
#include "../src/Substance.h"
#include "../src/DBClient/TraversalData.h"
#include "../src/Common/ParseBsonTraversalData.h"
#endif
#ifndef FROM_SRC
#include "thermofun/Batch/ThermoBatch.h"
#include "thermofun/Database.h"
#include "thermofun/Substance.h"
#include "thermofun/DBClient/TraversalData.h"
#include "thermofun/Common/ParseBsonTraversalData.h"
#endif

using namespace jsonio;
using namespace jsonui;

/*
bool operator !=( const DBQueryData& iEl,  const DBQueryData& iEr)
{
    return (iEl.getType() != iEr.getType()) ||
            (iEl.getQueryString() != iEr.getQueryString());
}*/

ThermoFunData::ThermoFunData()
{
  name = "ThermoFunTask1";
  comment = "write comment here...";
  calcStatus = "Select a thermodynamic dataset."; // status
  schemaName = "VertexSubstance";
  query = emptyQuery;
  idReactionSet = "";
  sourcetdb = -1;//defSourcetdb;
  tPrecision = 0;
  unitsT = "C Celsius";
  pPrecision = 0;
  tppairs.push_back({25.,1.});
  unitsP = "b bar";
  properties.push_back("gibbs_energy");
  propertyUnits.push_back("J/mol");
  propertyPrecision.push_back(0);
}


void ThermoFunData::resetSchemaName( const string& newSchemaName )
{
    schemaName = newSchemaName;
    query = emptyQuery;

    properties.resize(1);
    propertyUnits.resize(1);
    propertyPrecision.resize(1);

    if (schemaName == "VertexSubstance")
        properties[0] = "gibbs_energy";
    if (schemaName == "VertexReaction")
        properties[0] = "reaction_gibbs_energy";

    propertyUnits[0] = mapUnits[ properties[0]];
    propertyPrecision[0] = mapPrecision[ properties[0]];
}


void ThermoFunData::toJsonNode( jsonio::JsonDom *object ) const
{
    uint ii;
    object->appendString( "Name", name );
    object->appendString( "Description", comment );
    object->appendString( "SchemaName", schemaName );
    object->appendInt("SourceTDB", sourcetdb );
    object->appendString( "ReactionSetId", idReactionSet );
    if(!query.empty())
        query.toJsonNode(object);

    auto arr = object->appendArray("ElementsList");
    for( ii=0; ii<elements.size(); ii++)
    {
        auto arr2 = arr->appendArray( to_string(ii) );
        arr2->appendString( "0", elements[ii].symbol );
        arr2->appendInt( "1", elements[ii].class_ );
        arr2->appendInt( "2", elements[ii].isotope );
    }

    object->appendDouble("TemperaturePrecision", tPrecision );
    object->appendString("TemperatureUnits", unitsT );
    object->appendDouble("PressurePrecision", pPrecision );
    object->appendString("PressureUnits", unitsP );

    arr = object->appendArray( "TemperaturePressurePoints");
    for( ii=0; ii<tppairs.size(); ii++)
    {
      auto obj =  arr->appendObject( to_string(ii) );
        obj->appendDouble( "T", tppairs[ii][0] );
        obj->appendDouble( "P", tppairs[ii][1] );
    }

    arr = object->appendArray("PropertiesList");
    for( ii=0; ii<properties.size(); ii++)
       arr->appendString( to_string(ii), properties[ii] );

    arr = object->appendArray( "PropertyUnits");
    for( ii=0; ii<propertyUnits.size(); ii++)
       arr->appendString(  to_string(ii), propertyUnits[ii] );

    arr = object->appendArray( "PropertyPrecision");
    for( ii=0; ii<propertyPrecision.size(); ii++)
       arr->appendInt(  to_string(ii), propertyPrecision[ii] );
}

void ThermoFunData::fromJsonNode( const jsonio::JsonDom *object )
{
    ThermoFunData deflt;

    if(!object->findValue( "Name", name ) )
        name=deflt.name;
    if(!object->findValue( "Description", comment ) )
        comment=deflt.comment;
    if(!object->findValue( "SchemaName", schemaName ) )
        schemaName=deflt.schemaName;
    if(!object->findValue( "SourceTDB", sourcetdb ) )
        sourcetdb=deflt.sourcetdb;
    if(!object->findValue( "ReactionSetId", idReactionSet ) )
        idReactionSet="";
    query.fromJsonNode(object);

    elements.clear();
    auto arr  = object->field(  "ElementsList"  );
    if( arr!=nullptr && arr->getType() == JSON_ARRAY )
    {
       string symbol;
       int class_, isotope;
       for( int ii=0; ii<arr->getChildrenCount(); ii++ )
       {
           auto obj = arr->getChild( ii);
           if(!obj->findValue( "0", symbol ) )
             continue;
           if(!obj->findValue( "1", class_ ) )
             continue;
           if(!obj->findValue( "2", isotope ) )
             continue;
           elements.push_back( ThermoFun::ElementKey(symbol,class_, isotope ) );
       }
    }

    if(!object->findValue( "TemperaturePrecision", tPrecision ) )
        tPrecision=deflt.tPrecision;
    if(!object->findValue( "TemperatureUnits", unitsT ) )
        unitsT=deflt.unitsT;

    if(!object->findValue( "PressurePrecision", pPrecision ) )
        pPrecision=deflt.pPrecision;
    if(!object->findValue( "PressureUnits", unitsP ) )
        unitsP=deflt.unitsP;

    // read tp pairs
    tppairs.clear();
    arr  = object->field(  "TemperaturePressurePoints"  );
    if( arr==nullptr && arr->getType() != JSON_ARRAY )
      tppairs =deflt.tppairs;
    else
    {
       double Ti, Pi;
       for( int ii=0; ii<arr->getChildrenCount(); ii++ )
       {
         auto obj = arr->getChild( ii);
         if(!obj->findValue( "T", Ti ) )
            Ti=100.;
         if(!obj->findValue( "P", Pi ) )
            Pi = 5.;
         tppairs.push_back({Ti, Pi});
       }
    }

    object->findArray( "PropertiesList", properties );
    if( properties.empty() )
        properties =deflt.properties;
    object->findArray( "PropertyUnits", propertyUnits );
    if( properties.empty() )
        properties =deflt.properties;
    object->findArray( "PropertyPrecision", propertyPrecision );
    if( properties.empty() )
        properties =deflt.properties;
    }


ThermoFunWidgetPrivate::ThermoFunWidgetPrivate( ThermoFunWidgetNew* awindow ):
 QObject(awindow), window(awindow), dbclient( uiSettings().dbclient() )
{
    initWindow();
}

void ThermoFunWidgetPrivate::updateDBClient()
{
   /** dbclient.substData().updateDBClient();
   dbclient.reactData().updateDBClient();
   dbclient.reactSetData().updateDBClient();
   */
}

// Edit part ----------------------------------------------------

void ThermoFunWidgetPrivate::loadModel()
{
   if( !_data.idReactionSet.empty()  && dbclient.reactSetData().recordExists(_data.idReactionSet) )
    tableModel->loadModeRecords( _data.idReactionSet );
   else
    tableModel->loadModeRecords( _data.query, _data.sourcetdb, _data.elements );
}

/// Reset internal query data
void ThermoFunWidgetPrivate::updateQuery( const DBQueryData& newquery  )
{
  // reset internal query data
  if( newquery != _data.query )
  {
     isDefaultQuery = true;
     _data.query = newquery;
   }
   loadModel();
}

/// Reset internal elements list
void ThermoFunWidgetPrivate::updateElements( int sourcetdb,
     const vector<ThermoFun::ElementKey>& elKeys, const string& idrcset )
{
        _data.idReactionSet = idrcset;
        _data.sourcetdb = sourcetdb;
        _data.elements  = elKeys;
         updateElementsModel();
         loadModel();
}

void ThermoFunWidgetPrivate::updataSourceTDB()
{
    jsonio::ThriftEnumDef* enumdef =  ioSettings().Schema()->getEnum("SourceTDB" );
    if(enumdef != nullptr )
    {
        string curText = enumdef->getNamebyId(_data.sourcetdb);
        window->ui->pSourceTDB->setCurrentText(curText.c_str());
    }
}

void ThermoFunWidgetPrivate::updateElementsModel()
{
    if( elementModel)
    {
      elementModel->resetMatrixData();
      if( elementTable->horizontalScrollBar()->isVisible() )
      {
          int iHeight = elementTable->verticalHeader()->sectionSize(0);
          iHeight += elementTable->horizontalScrollBar()->height();
          elementTable->setMaximumHeight(iHeight);
      }
    }
    updataSourceTDB();
}

void ThermoFunWidgetPrivate::reallocTP( int newsize )
{
  _data.tppairs.clear();
   for (uint i = 0; i<newsize; i++ )
       _data.tppairs.push_back({0,0});
  _TPlistModel->resetMatrixData();
}

void ThermoFunWidgetPrivate::updateTP( const string& unitsT, const string& unitsP,
                   const std::vector<std::vector<double>>& tppairs)
{
    _data.unitsT = unitsT;
    _data.unitsP = unitsP;
    window->ui->pTunit->setCurrentText(unitsT.c_str());
    window->ui->pPunit->setCurrentText(unitsP.c_str());
    _data.tppairs = tppairs;
   _TPlistModel->resetMatrixData();
}


void ThermoFunWidgetPrivate::updateProperty( const vector<string>& properties )
{
    // store previous settings
    for (unsigned i = 0; i<_data.properties.size(); i++)
    {
        _data.mapUnits[_data.properties[i]] = _data.propertyUnits[i];
        _data.mapPrecision[_data.properties[i]] = _data.propertyPrecision[i];
    }
    _data.properties = properties;
    _data.propertyUnits.resize(_data.properties.size());
    _data.propertyPrecision.resize(_data.properties.size());
    for (unsigned i = 0; i<_data.properties.size(); i++)
    {
        _data.propertyUnits[i] = _data.mapUnits[_data.properties[i]];
        _data.propertyPrecision[i] = _data.mapPrecision[_data.properties[i]];
    }
   _PropertyModel->resetMatrixData();
}


void ThermoFunWidgetPrivate::linkChange()
{
  if( _data.schemaName == "VertexReaction")
  {
      tableModel->linkData(new ThermoFun::SubstanceData_(dbclient.substData()));
      loadModel();
      tableModel->linkData(new ThermoFun::ReactionData_(dbclient.reactData()));
      loadModel();
  }
  else
  {
      tableModel->linkData(new ThermoFun::ReactionData_(dbclient.reactData()));
      loadModel();
      tableModel->linkData(new ThermoFun::SubstanceData_(dbclient.substData()));
      loadModel();
  }
}

void ThermoFunWidgetPrivate::typeChanged(const string& newSchemaName)
{
  if( newSchemaName != _curSchemaName )
  {
     _curSchemaName = newSchemaName;
     _data.resetSchemaName( _curSchemaName );
     _PropertyModel->resetMatrixData();
     isDefaultQuery = !_data.query.empty();
     window->ui->edgeQuery->setText(_data.query.getQueryString().c_str());
  }
  linkChange();
}

// Reset ThermoFun data
void ThermoFunWidgetPrivate::newThermoFunData( const ThermoFunData& newdata )
{
    _data = newdata;

    tableModel->linkData(new ThermoFun::SubstanceData_(dbclient.substData()));

    _curSchemaName = _data.schemaName;
    // update models
    updateElementsModel();
   _TPlistModel->resetMatrixData();
   _PropertyModel->resetMatrixData();
//    linkChange();
    isDefaultQuery = !_data.query.empty();
    // update view must be up
}


// Calc part ------------------------------

// extract init for calculation data
void ThermoFunWidgetPrivate::loadSubstData( const vector<int>& selNdx,
  vector<string>& aKeyList, vector<string>& substancesSymbols,
  vector<string>& substancesClass )
{
    if(_data.schemaName != "VertexSubstance")
       return;

    const jsonio::ValuesTable& values= tableModel->getValues();
    aKeyList.resize(selNdx.size());
    substancesSymbols.resize(selNdx.size());
    substancesClass.resize(selNdx.size());

    for( uint ii=0; ii<selNdx.size(); ii++ )
     {
        auto itValues = values[selNdx[ii]];
        substancesSymbols[ii] = itValues[0];
        substancesClass[ii] = itValues[4];
        aKeyList[ii] = itValues[3]/*+":"*/;
     }
}

void ThermoFunWidgetPrivate::loadReactData( const vector<int>& selNdx,
  vector<string>& aKeyList, vector<string>& reactionsSymbols )
{
    if (_data.schemaName != "VertexReaction")
      return;
    const jsonio::ValuesTable& values= tableModel->getValues();
    aKeyList.resize(selNdx.size());
    reactionsSymbols.resize(selNdx.size());
    for( uint ii=0; ii<selNdx.size(); ii++ )
    {
        auto itValues = values[selNdx[ii]];
        reactionsSymbols[ii] = itValues[0];
        aKeyList[ii] = itValues[3]/*+":"*/;
    }
}

void ThermoFunWidgetPrivate::retrieveConnectedData(ThermoFun::VertexId_VertexType mapOfConnectedIds,
                            vector<string> &linkedSubstSymbols, vector<string> &linkedReactSymbols,
                                                   vector<string> &linkedSubstClasses, vector<string> &linkedSubstIds)
{
    string valDB;
    for (auto idType : mapOfConnectedIds)
    {
        if (idType.second == "reaction")
        {
            string symbol;
            string jsonrecord = dbclient.reactData().getJsonRecordVertex(idType.first+":");
            auto domdata = jsonio::unpackJson( jsonrecord );
            domdata->findKey("properties.symbol",  symbol);
            // record = dbclient.reactData().getJsonRecordVertex(idType.first+":").second;
            // jsonio::bson_to_key( record.data, "properties.symbol",  symbol);
            linkedReactSymbols.push_back(symbol);
        }
        if (idType.second == "substance")
        {
            string symbol, class_;
            string jsonrecord = dbclient.substData().getJsonRecordVertex(idType.first+":");
            auto domdata = jsonio::unpackJson( jsonrecord );
            domdata->findKey("properties.symbol",  symbol);
            domdata->findKey("properties.class_",  class_);
            //  record = dbclient.substData().getJsonBsonRecordVertex(idType.first+":").second;
            //  jsonio::bson_to_key( record.data, "properties.symbol",  symbol);
            //  jsonio::bson_to_key( record.data, "properties.class_",  class_);
            linkedSubstClasses.push_back(class_);
            linkedSubstSymbols.push_back(symbol);
            linkedSubstIds.push_back(idType.first);
        }
    }
}

void ThermoFunWidgetPrivate::retrieveConnectedDataSymbols( const vector<string>& substKeys, const vector<string>& reactKeys, vector<string> &linkedSubstSymbols,
                                                           vector<string> &linkedReactSymbols, vector<string> &linkedSubstClasses, vector<string> &linkedSubstIds)
{
    linkedReactSymbols.clear(); linkedSubstSymbols.clear(); linkedSubstClasses.clear();

    retrieveConnectedData(dbclient.getTraversal().getMapOfConnectedIds(substKeys), linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses, linkedSubstIds);
    retrieveConnectedData(dbclient.getTraversal().getMapOfConnectedIds(reactKeys), linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses, linkedSubstIds);
//    react.getLinkedDataSymbols(reactKeys, linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses);
//    subst.getLinkedDataSymbols(substKeys, linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses);
}

void ThermoFunWidgetPrivate::setSubstanceLevel(string substSymbol, string level)
{
    dbclient.substData().setSubstanceLevel(substSymbol, level);
}

//void ThermoFunWidgetPrivate::setReactionLevel(string reactSymbol, string level)
//{
//    react.setReactionLevel(reactSymbol, level);
//}

MapSymbolMapLevelReaction ThermoFunWidgetPrivate::recordsMapLevelDefinesReaction_(ThermoFun::MapSubstSymbol_MapLevel_IdReaction mapSymbol_Level_idReact)
{
    MapSymbolMapLevelReaction recordsLevelReact;

    for (auto symbol_Level_idReact : mapSymbol_Level_idReact)
    {
        MapLevelReaction levelReact;
        for (auto level_idReact : symbol_Level_idReact.second)
        {
            levelReact[level_idReact.first] = ThermoFun::parseReaction(
                        dbclient.reactData().getJsonRecordVertex(level_idReact.second+":") );
        }
        recordsLevelReact[symbol_Level_idReact.first] = levelReact;
    }
    return recordsLevelReact;
}

MapSymbolMapLevelReaction ThermoFunWidgetPrivate::recordsMapLevelDefinesReaction()
{
    return recordsMapLevelDefinesReaction_(dbclient.substData().recordsMapLevelDefinesReaction());
}

MapSymbolMapLevelReaction ThermoFunWidgetPrivate::recordsMapLevelDefinesReaction(vector<string> connectedSubstIds , vector<string> connectedSubstSymbols)
{
    return recordsMapLevelDefinesReaction_(dbclient.substData().recordsMapLevelDefinesReaction(connectedSubstIds, connectedSubstSymbols));
}

ThermoFun::Database setSubstanceCalcType_ (ThermoFun::Database tdb, ThermoFun::SubstanceThermoCalculationType::type type_)
{
    auto substances = tdb.getSubstances();
    for (auto substance : substances)
    {
        substance.setThermoCalculationType(type_);
        tdb.setSubstance(substance);
    }
    return tdb;
}

//MapSymbolMapLevelSubstances ThermoFunWidgetPrivate::recordsMapLevelTakesSubstances()
//{
//    return react.recordsMapLevelTakesSubstances();
//}

double ThermoFunWidgetPrivate::calcData(const vector<string>& substKeys, const vector<string>& reactKeys,
  const vector<string>& substancesSymbols,  const vector<string>& reactionsSymbols,
  const string& solventSymbol, bool FormatBox , bool calcSubstFromReact, bool calcReactFromSubst, struct timeval start)
{
    vector<string> keys; keys.insert(keys.end(), substKeys.begin(), substKeys.end());
                         keys.insert(keys.end(), reactKeys.begin(), reactKeys.end());

    auto tr = dbclient.getTraversal();

    ThermoFun::VertexId_VertexType resultTraversal = tr.getMapOfConnectedIds(keys, dbclient.substData().getSubstSymbol_DefinesLevel());
    ThermoFun::Database tdb_ = tr.getDatabaseFromMapOfIds(resultTraversal, dbclient.substData().getSubstSymbol_DefinesLevel());

//    ThermoFun::Traversal tr(subst.getDB());
//    ThermoFun::MapIdType resultTraversal = tr.getMapOfConnectedIds(keys, subst.mapSymbolLevel());
//    ThermoFun::Database tdb_ = tr.getDatabaseFromMapOfIds(resultTraversal, subst.mapSymbolLevel());

    if (!calcSubstFromReact) // make all reactions to be calculated using the method in the record
        tdb_ = setSubstanceCalcType_(tdb_, ThermoFun::SubstanceThermoCalculationType::type::DCOMP);


    ThermoFun::ThermoBatch tpCalc (tdb_);
    tpCalc.setSolventSymbol(solventSymbol);

    ThermoFun::OutputSettings op;
    if( FormatBox )
    {
      op.isFixed = true;
    } else
        op.isFixed = false;

    op.outSolventProp       = true;
    op.calcReactFromSubst   = calcReactFromSubst;
    op.calcSubstFromReact   = calcSubstFromReact;
    tpCalc.setOutputSettings(op);

    tpCalc.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

    if (_data.unitsP == "B kbar")
        tpCalc.setPropertyUnit("pressure","kbar");
    if (_data.unitsP == "p Pa")
        tpCalc.setPropertyUnit("pressure","Pa");
    if (_data.unitsP == "P MPa")
        tpCalc.setPropertyUnit("pressure","MPa");
    if (_data.unitsP == "A Atm")
        tpCalc.setPropertyUnit("pressure","atm");

    /// check !!!
    vector<string> solventPropNames = {"density", "alpha", "beta", "alphaT", "epsilon", "bornZ", "bornY", "bornQ", "bornX"};
//    tpCalc.addSolventProperties(solventPropNames);

    std::map<std::string, int> precision = ThermoFun::defaultPropertyDigits;
    for (uint jj = 0; jj <_data.properties.size(); jj++)
    {
        precision.at(_data.properties[jj]) = _data.propertyPrecision[jj];
    }

    precision.at("temperature") = _data.tPrecision;
    precision.at("pressure")    = _data.pPrecision;

    tpCalc.setDigits(precision);

    if (_data.schemaName == "VertexReaction")
    {
        tpCalc.thermoPropertiesReaction(_data.tppairs, reactionsSymbols, _data.properties/*, calcReactFromSubst*/).toCSV(op.fileName);

        vector<string> reactionsEquations;
        for (auto symb : reactionsSymbols)
        {
            reactionsEquations.push_back(tdb_.getReaction(symb).equation());
        }

        /// check !!!
//        tpCalc.clearReactions();
//        tpCalc.addReactions(reactionsEquations);

//        ThermoFun::Output (tpCalc).toCSV("equations.csv");
    }


    if (_data.schemaName == "VertexSubstance")
        tpCalc.thermoPropertiesSubstance( _data.tppairs, substancesSymbols, _data.properties/*, calcSubstFromReact*/).toCSV(op.fileName);

    struct timeval end;
    gettimeofday(&end, NULL);
    double delta_calc = ((end.tv_sec  - start.tv_sec) * 1000000u +
                     end.tv_usec - start.tv_usec) / 1.e6;

     return delta_calc;

}


//-----------------------------------------------------------------------

void ThermoFunWidgetPrivate::initWindow()
{
    // define all keys tables
    valuesTable = new TMatrixTable( window );
    valuesTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    valuesTable->setEditTriggers( QAbstractItemView::AnyKeyPressed );
    valuesTable->setSortingEnabled(true);
    QObject::disconnect( valuesTable, SIGNAL(customContextMenuRequested(QPoint)),
           valuesTable, SLOT(slotPopupContextMenu(QPoint)));
    //TMatrixDelegate* deleg = new TMatrixDelegate();
    //pTable->setItemDelegate(deleg);
    window->ui->keySplitter->insertWidget(0, valuesTable);
    window->ui->keySplitter->setStretchFactor(0, 4);
    tableModel = new ThermoViewModel( new ThermoFun::SubstanceData_(dbclient.substData()), window );
    tableModel->updateModel(valuesTable);

    // Elements list
    elementContainer = new ElementsDataContainer( _data.elements );
    elementModel = new TMatrixModel( elementContainer, window );
    elementTable = new TMatrixTable( window, TMatrixTable::tbShow );
    elementTable->setModel(elementModel);
    elementTable->horizontalHeader()->hide();
    elementTable->verticalHeader()->hide();
    elementTable->verticalHeader()->setStretchLastSection(true);
    //elementTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    int iHeight = 1.5*elementTable->verticalHeader()->sectionSize(0);
    //iHeight += elementTable->horizontalScrollBar()->height();
    elementTable->setMaximumHeight(iHeight);
    window->ui->verticalLayout_2->insertWidget(0, elementTable);

   // define ThermoFun data
   _TPContainer = new TPContainer( "T", { "T", "P" }, _data.tppairs );
   _TPlistTable  = new TMatrixTable( window->ui->outWidget );
   TMatrixDelegate* deleg = new TMatrixDelegate();
   _TPlistTable->setItemDelegate(deleg);
   _TPlistModel = new TMatrixModel( _TPContainer, window );
   _TPlistTable->setModel(_TPlistModel);
   _TPlistTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch/*Interactive*/ );
   window->ui->gridLayout_3->addWidget(_TPlistTable, 1, 0, 1, 1);

   _PropertyContainer = new TPropertyContainer( "Property", _data.properties, _data.propertyUnits, _data.propertyPrecision );
   _PropertyTable  = new TMatrixTable( window->ui->inWidget );
    deleg = new TMatrixDelegate();
   _PropertyTable->setItemDelegate(deleg);
   _PropertyModel = new TMatrixModel( _PropertyContainer, window );
   _PropertyTable->setModel(_PropertyModel);
   _PropertyTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch/*Interactive*/ );
    window->ui->gridLayout->addWidget(_PropertyTable, 7, 0, 1, 8);
}

void ThermoFunWidgetPrivate::freeInternal()
{
    delete  tableModel;
    delete  valuesTable;
    delete _TPlistTable;
    delete _TPContainer;
    delete _PropertyTable;
    delete _PropertyContainer;
    delete elementContainer;
    delete elementTable;
}

