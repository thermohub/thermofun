#ifdef _WIN32
#else
#include <sys/time.h>
#endif
#include <QScrollBar>
#include <QHeaderView>
#include "ThermoFunPrivateNew.h"
#include "ui_ThermoFunWidget.h"

// ThermoFun includes
#ifdef FROM_SRC
#include "../src/Database.h"
#include "../src/DBClient/TraversalData.h"
#include "../src/DBClient/ReactionData.h"
#include "../src/DBClient/ReactionSetData.h"
#include "../src/Batch/OutputBatch.h"
#include "../src/Common/ParseBsonTraversalData.h"
#else
#include "thermofun/Database.h"
#include "thermofun/DBClient/TraversalData.h"
#include "thermofun/DBClient/ReactionData.h"
#include "thermofun/DBClient/ReactionSetData.h"
#include "thermofun/Interfaces/Output.h"
#include "thermofun/Common/ParseBsonTraversalData.h"
#endif

ThermoFunData::ThermoFunData()
{
  name = "ThermoFunTask1";
  comment = "write comment here...";
  calcStatus = "Select a thermodynamic dataset."; // status
  schemaName = "VertexSubstance";
  idThermoDataSet = "";
  sourceTDBs.clear();

  tPrecision = 0;
  unitsT = "C Celsius";
  pPrecision = 0;
  tppairs.push_back({25.,1.});
  unitsP = "b bar";
  properties.push_back("gibbs_energy");
  propertyUnits.push_back("J/mol");
  propertyPrecision.push_back(0);

  // old
  _query = jsonio::emptyQuery;
  _idReactionSet = "";
  _sourcetdb = -1;//defSourcetdb;
}

void ThermoFunData::resetSchemaName( const string& newSchemaName )
{
    schemaName = newSchemaName;
    properties.resize(1);
    propertyUnits.resize(1);
    propertyPrecision.resize(1);

    if (schemaName == "VertexSubstance")
        properties[0] = "gibbs_energy";
    if (schemaName == "VertexReaction")
        properties[0] = "reaction_gibbs_energy";

    propertyUnits[0] = mapUnits[ properties[0]];
    propertyPrecision[0] = mapPrecision[ properties[0]];

    // old
    _query = jsonio::emptyQuery;
}


void ThermoFunData::toJsonNode( jsonio::JsonDom *object ) const
{
    unsigned int ii;
    object->appendString( "Name", name );
    object->appendString( "Description", comment );
    object->appendString( "SchemaName", schemaName );

    object->appendString( "ThermoDataSetId", idThermoDataSet );
    auto arr = object->appendArray("SourceTDBs");
    for( ii=0; ii<sourceTDBs.size(); ii++)
        arr->appendInt(to_string(ii), sourceTDBs[ii] );

    arr = object->appendArray("ElementsList");
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

    // old
    object->appendInt("SourceTDB", _sourcetdb );
    object->appendString( "ReactionSetId", _idReactionSet );
    if(!_query.empty())
        _query.toJsonNode(object);

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

    if(!object->findValue( "ThermoDataSetId", idThermoDataSet ) )
        idThermoDataSet="";
    object->findArray( "SourceTDBs", sourceTDBs );

    elements.clear();
    auto arr  = object->field(  "ElementsList"  );
    if( arr!=nullptr && arr->getType() == jsonio::JSON_ARRAY )
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
    if( arr==nullptr && arr->getType() != jsonio::JSON_ARRAY )
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

    // old
    if(!object->findValue( "SourceTDB", _sourcetdb ) )
        _sourcetdb=deflt._sourcetdb;
    if(!object->findValue( "ReactionSetId", _idReactionSet ) )
        _idReactionSet="";
    _query.fromJsonNode(object);

}

 std::string ThermoFunData::selection_to_string() const
 {
     std::string output;
     output +=  " idThermoDataSet: " + idThermoDataSet;
     output +=  "\n sourceTDBs: ";

     jsonio::ThriftEnumDef* enumdef =  jsonio::ioSettings().Schema()->getEnum( "SourceTDB" );
     if(enumdef != nullptr )
     {
      for( unsigned int ii=0; ii<sourceTDBs.size(); ii++ )
              output += enumdef->getNamebyId(sourceTDBs[ii]) + "  ";
     }
     output +=  "\n";
     return output;
 }

// ThermoFunPrivateNew ------------------------------------------------------------------

ThermoFunPrivateNew::ThermoFunPrivateNew( ThermoFunWidgetNew* awindow ):
 QObject(awindow), window(awindow), dbclient( jsonui::uiSettings().dbclient() )
{
    initWindow();
}

void ThermoFunPrivateNew::freeInternal()
{
    delete _TPlistTable;
    delete _TPContainer;
    delete _PropertyTable;
    delete _PropertyContainer;
    delete elementContainer;
    delete elementTable;
}

void ThermoFunPrivateNew::initWindow()
{
    // define all keys tables
    valuesTable = new jsonui::TMatrixTable( window );
    valuesTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    valuesTable->setEditTriggers( QAbstractItemView::AnyKeyPressed );
    valuesTable->setSortingEnabled(true);
    QObject::disconnect( valuesTable, SIGNAL(customContextMenuRequested(QPoint)),
           valuesTable, SLOT(slotPopupContextMenu(QPoint)));
    window->ui->keySplitter->insertWidget(0, valuesTable);
    window->ui->keySplitter->setStretchFactor(0, 4);
    substModel.reset(new ThermoViewModel( &dbclient.substData(), window ));
    reactModel.reset(new ThermoViewModel( &dbclient.reactData(), window ));
    substModel->updateModel(valuesTable);

    // Elements list
    elementContainer = new ElementsDataContainer( _data.elements );
    elementModel = new jsonui::TMatrixModel( elementContainer, window );
    elementTable = new jsonui::TMatrixTable( window, jsonui::TMatrixTable::tbShow );
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
   _TPlistTable  = new jsonui::TMatrixTable( window->ui->outWidget );
   jsonui::TMatrixDelegate* deleg = new jsonui::TMatrixDelegate();
   _TPlistTable->setItemDelegate(deleg);
   _TPlistModel = new jsonui::TMatrixModel( _TPContainer, window );
   _TPlistTable->setModel(_TPlistModel);
   _TPlistTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch/*Interactive*/ );
   window->ui->gridLayout_3->addWidget(_TPlistTable, 1, 0, 1, 1);

   _PropertyContainer = new TPropertyContainer( "Property", _data.properties, _data.propertyUnits, _data.propertyPrecision );
   _PropertyTable  = new jsonui::TMatrixTable( window->ui->inWidget );
    deleg = new jsonui::TMatrixDelegate();
   _PropertyTable->setItemDelegate(deleg);
   _PropertyModel = new jsonui::TMatrixModel( _PropertyContainer, window );
   _PropertyTable->setModel(_PropertyModel);
   _PropertyTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch/*Interactive*/ );
    window->ui->gridLayout->addWidget(_PropertyTable, 7, 0, 1, 8);
}


// Edit part --------------------------------------------------------------------


// Reset internal data
void ThermoFunPrivateNew::updateData( const std::string& aThermoDataSet,
                                      const std::vector<int>& sourcetdbs,
                                      const std::vector<ThermoFun::ElementKey>& elementKeys,
                                      const jsonio::ValuesTable&  substanceValues,
                                      const jsonio::ValuesTable&  reactionValues )
{
   _data.idThermoDataSet = aThermoDataSet;
   _data.sourceTDBs = move(sourcetdbs);
   _data.elements  = elementKeys;
    substModel->loadModeRecords( substanceValues );
    reactModel->loadModeRecords( reactionValues );
    updateElementsModel();

}

void ThermoFunPrivateNew::updateElementsModel()
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
    updateSelectMessage();
}

void ThermoFunPrivateNew::updateSelectMessage()
{
    window->ui->taskQuery->setText( _data.selection_to_string().c_str() );
}

void ThermoFunPrivateNew::reallocTP( int newsize )
{
  _data.tppairs.clear();
   for (unsigned int i = 0; i<newsize; i++ )
       _data.tppairs.push_back({0,0});
  _TPlistModel->resetMatrixData();
}

void ThermoFunPrivateNew::updateTP( const string& unitsT, const string& unitsP,
                   const std::vector<std::vector<double>>& tppairs)
{
    _data.unitsT = unitsT;
    _data.unitsP = unitsP;
    window->ui->pTunit->setCurrentText(unitsT.c_str());
    window->ui->pPunit->setCurrentText(unitsP.c_str());
    _data.tppairs = tppairs;
   _TPlistModel->resetMatrixData();
}


void ThermoFunPrivateNew::updateProperty( const vector<string>& properties )
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


void ThermoFunPrivateNew::linkChange()
{
  if( _data.schemaName == "VertexReaction")
  {
      reactModel->updateModel(valuesTable);
  }
  else
  {
      substModel->updateModel(valuesTable);
  }
}

void ThermoFunPrivateNew::typeChanged(const string& newSchemaName)
{
  if( newSchemaName != _curSchemaName )
  {
     _curSchemaName = newSchemaName;
     _data.resetSchemaName( _curSchemaName );
     _PropertyModel->resetMatrixData();
     //isDefaultQuery = !_data.query.empty();
     //window->ui->edgeQuery->setText(_data.query.getQueryString().c_str());
     linkChange();
  }
}

// Reset ThermoFun data
void ThermoFunPrivateNew::newThermoFunData( const ThermoFunData& newdata )
{
    _data = newdata;

   _curSchemaName = _data.schemaName;
    // update models
    updateElementsModel();
   _TPlistModel->resetMatrixData();
   _PropertyModel->resetMatrixData();
    linkChange();
    //isDefaultQuery = !_data.query.empty();
    // clear selection lists
    jsonio::ValuesTable emptytable;
    substModel->loadModeRecords( emptytable );
    reactModel->loadModeRecords( emptytable );
}


// Calc part ------------------------------

// extract init for calculation data
void ThermoFunPrivateNew::loadSubstData( const vector<int>& selNdx,
  vector<string>& aKeyList, vector<string>& substancesSymbols,
  vector<string>& substancesClass )
{
    //if(_data.schemaName != "VertexSubstance")
    //   return;

    const jsonio::ValuesTable& values= substModel->getValues();
    aKeyList.resize(selNdx.size());
    substancesSymbols.resize(selNdx.size());
    substancesClass.resize(selNdx.size());

    for( unsigned int ii=0; ii<selNdx.size(); ii++ )
     {
        auto itValues = values[selNdx[ii]];
        substancesSymbols[ii] = itValues[0];
        substancesClass[ii] = itValues[4];
        aKeyList[ii] = itValues[3];
     }
}

void ThermoFunPrivateNew::loadReactData( const vector<int>& selNdx,
  vector<string>& aKeyList, vector<string>& reactionsSymbols )
{
    //if (_data.schemaName != "VertexReaction")
    //  return;
    const jsonio::ValuesTable& values= reactModel->getValues();
    aKeyList.resize(selNdx.size());
    reactionsSymbols.resize(selNdx.size());
    for( unsigned int ii=0; ii<selNdx.size(); ii++ )
    {
        auto itValues = values[selNdx[ii]];
        reactionsSymbols[ii] = itValues[0];
        aKeyList[ii] = itValues[3];
    }
}

void ThermoFunPrivateNew::retrieveConnectedData(ThermoFun::VertexId_VertexType mapOfConnectedIds,
                            vector<string> &linkedSubstSymbols, vector<string> &linkedReactSymbols,
                                                   vector<string> &linkedSubstClasses, vector<string> &linkedSubstIds)
{
    string valDB;
    for (auto idType : mapOfConnectedIds)
    {
        if (idType.second == "reaction")
        {
            string symbol;
            string jsonrecord = dbclient.reactData().getJsonRecordVertex(idType.first);
            auto domdata = jsonio::unpackJson( jsonrecord );
            domdata->findValue("properties.symbol",  symbol);
            // record = dbclient.reactData().getJsonRecordVertex(idType.first).second;
            // jsonio::bson_to_key( record.data, "properties.symbol",  symbol);
            linkedReactSymbols.push_back(symbol);
        }
        if (idType.second == "substance")
        {
            string symbol, class_;
            string jsonrecord = dbclient.substData().getJsonRecordVertex(idType.first);
            auto domdata = jsonio::unpackJson( jsonrecord );
            domdata->findValue("properties.symbol",  symbol);
            domdata->findValue("properties.class_",  class_);
            //  record = dbclient.substData().getJsonBsonRecordVertex(idType.first).second;
            //  jsonio::bson_to_key( record.data, "properties.symbol",  symbol);
            //  jsonio::bson_to_key( record.data, "properties.class_",  class_);
            /// !!!! class now struct
            linkedSubstClasses.push_back(class_);
            linkedSubstSymbols.push_back(symbol);
            linkedSubstIds.push_back(idType.first);
        }
    }
}

void ThermoFunPrivateNew::retrieveConnectedDataSymbols( const vector<string>& substKeys, const vector<string>& reactKeys, vector<string> &linkedSubstSymbols,
                                                           vector<string> &linkedReactSymbols, vector<string> &linkedSubstClasses, vector<string> &linkedSubstIds)
{
    linkedReactSymbols.clear(); linkedSubstSymbols.clear(); linkedSubstClasses.clear();

    retrieveConnectedData(dbclient.getTraversal().getMapOfConnectedIds(substKeys), linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses, linkedSubstIds);
    retrieveConnectedData(dbclient.getTraversal().getMapOfConnectedIds(reactKeys), linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses, linkedSubstIds);
//    react.getLinkedDataSymbols(reactKeys, linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses);
//    subst.getLinkedDataSymbols(substKeys, linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses);
}

void ThermoFunPrivateNew::setSubstanceLevel(string substSymbol, string level)
{
    dbclient.substData().setSubstanceLevel(substSymbol, level);
}

//void ThermoFunWidgetPrivate::setReactionLevel(string reactSymbol, string level)
//{
//    react.setReactionLevel(reactSymbol, level);
//}

MapSymbolMapLevelReaction ThermoFunPrivateNew::recordsMapLevelDefinesReaction_(ThermoFun::MapSubstSymbol_MapLevel_IdReaction mapSymbol_Level_idReact)
{
    MapSymbolMapLevelReaction recordsLevelReact;

    for (auto symbol_Level_idReact : mapSymbol_Level_idReact)
    {
        MapLevelReaction levelReact;
        for (auto level_idReact : symbol_Level_idReact.second)
        {
            levelReact[level_idReact.first] = ThermoFun::parseReaction(
                        dbclient.reactData().getJsonRecordVertex(level_idReact.second) );
        }
        recordsLevelReact[symbol_Level_idReact.first] = levelReact;
    }
    return recordsLevelReact;
}

MapSymbolMapLevelReaction ThermoFunPrivateNew::recordsMapLevelDefinesReaction()
{
    return recordsMapLevelDefinesReaction_(dbclient.substData().recordsMapLevelDefinesReaction());
}

MapSymbolMapLevelReaction ThermoFunPrivateNew::recordsMapLevelDefinesReaction(vector<string> connectedSubstIds , vector<string> connectedSubstSymbols)
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

ThermoLoadData ThermoFunPrivateNew::loadData( vector<int> selNdx )
{
    ThermoLoadData data;

    try{

        if( isSubstances() )
            loadSubstData( selNdx, data.substKeys, data.substancesSymbols, data.substancesClass );
        else
            loadReactData( selNdx, data.reactKeys, data.reactionsSymbols );

        retrieveConnectedDataSymbols( data.substKeys, data.reactKeys, data.linkedSubstSymbols,
                                      data.linkedReactSymbols, data.linkedSubstClasses, data.linkedSubstIds);
    }
    catch(std::exception& e)
    {
        data.errorMessage = e.what();
    }
    return std::move(data);
}

string ThermoFunPrivateNew::calcData( ThermoLoadData loadedData, string solventSymbol,
                                      bool FormatBox, bool calcSubstFromReact, bool calcReactFromSubst )
{
    string returnMessage;
    QTime time;
    time.start();

    try{

        vector<string> keys;

        keys.insert(keys.end(), loadedData.substKeys.begin(), loadedData.substKeys.end());
        keys.insert(keys.end(), loadedData.reactKeys.begin(), loadedData.reactKeys.end());

        auto tr = dbclient.getTraversal();

        ThermoFun::VertexId_VertexType resultTraversal = tr.getMapOfConnectedIds(keys, dbclient.substData().getSubstSymbol_DefinesLevel());
        ThermoFun::Database tdb_ = tr.getDatabaseFromMapOfIds(resultTraversal, dbclient.substData().getSubstSymbol_DefinesLevel());

        //    ThermoFun::Traversal tr(subst.getDB());
        //    ThermoFun::MapIdType resultTraversal = tr.getMapOfConnectedIds(keys, subst.mapSymbolLevel());
        //    ThermoFun::Database tdb_ = tr.getDatabaseFromMapOfIds(resultTraversal, subst.mapSymbolLevel());

        if (!calcSubstFromReact) // make all reactions to be calculated using the method in the record
            tdb_ = setSubstanceCalcType_(tdb_, ThermoFun::SubstanceThermoCalculationType::type::DCOMP);


        ThermoFun::ThermoBatch batchCalc (tdb_);
        batchCalc.setSolventSymbol(solventSymbol);

        ThermoFun::BatchPreferences op;
        if( FormatBox )
        {
            op.isFixed = true;
        } else
            op.isFixed = false;

        op.outSolventProp       = true;
        op.calcReactFromSubst   = calcReactFromSubst;
        op.calcSubstFromReact   = calcSubstFromReact;
        batchCalc.setBatchPreferences(op);

        batchCalc.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

        if (_data.unitsP == "B kbar")
            batchCalc.setPropertyUnit("pressure","kbar");
        if (_data.unitsP == "p Pa")
            batchCalc.setPropertyUnit("pressure","Pa");
        if (_data.unitsP == "P MPa")
            batchCalc.setPropertyUnit("pressure","MPa");
        if (_data.unitsP == "A Atm")
            batchCalc.setPropertyUnit("pressure","atm");

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

        batchCalc.setDigits(precision);

        if (_data.schemaName == "VertexReaction")
        {
            batchCalc.thermoPropertiesReaction(_data.tppairs, loadedData.reactionsSymbols,
                                               _data.properties/*, calcReactFromSubst*/).toCSV(op.fileName);

            vector<string> reactionsEquations;
            for (auto symb : loadedData.reactionsSymbols)
            {
                reactionsEquations.push_back(tdb_.getReaction(symb).equation());
            }

            /// check !!!
            //        tpCalc.clearReactions();
            //        tpCalc.addReactions(reactionsEquations);

            //        ThermoFun::Output (tpCalc).toCSV("equations.csv");
        }

        if (_data.schemaName == "VertexSubstance")
            batchCalc.thermoPropertiesSubstance( _data.tppairs, loadedData.substancesSymbols,
                                                 _data.properties/*, calcSubstFromReact*/).toCSV(op.fileName);

        double delta_calc = time.elapsed()+ loadedData.time;
        returnMessage = "Calculation finished ("+ to_string(delta_calc/1000) + "s). Click view results.";
    }
    catch(std::exception& e)
    {
        returnMessage = string("Calculation finished with error: \n") + e.what();
    }
    return returnMessage;
}

//-----------------------------------------------------------------------

// temporaly functions
void ThermoFunPrivateNew::resetElementsintoRecord( bool isreact, const string& aKey )
{
  if(isreact)
   dbclient.reactData().resetRecordElements( aKey );
  else
   dbclient.reactSetData().resetRecordElements( aKey );
}

