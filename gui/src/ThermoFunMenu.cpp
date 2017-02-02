//  This is ThermoFun library+API (https://bitbucket.org/gems4/ThermoFun)
//
/// \file ThermoFunMenu.cpp
/// ThermoFunWidget - Widget to work with ThermoFun data
//
// BSONUI is a C++ Qt5-based widget library and API aimed at implementing
// the GUI for editing/viewing the structured data kept in a NoSQL database,
// with rendering documents to/from JSON/YAML/XML files, and importing/
// exporting data from/to arbitrary foreign formats (csv etc.). Graphical
// presentation of tabular data (csv format fields) is also possible.
//
// Copyright (c) 2015-2016 Svetlana Dmytriieva (svd@ciklum.com) and
//   Dmitrii Kulik (dmitrii.kulik@psi.ch)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU (Lesser) General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// BSONUI depends on the following open-source software products:
// BSONIO (https://bitbucket.org/gems4/bsonio); Qt5 (https://qt.io);
// Qwtplot (http://qwt.sourceforge.net).

#include <QMessageBox>
#include <QLineEdit>
#include <QInputDialog>
#include <sys/time.h>
#include <algorithm>
#include "ThermoFunWidget.h"
#include "ui_ThermoFunWidget.h"
#include "TPSetDialog.h"
#include "bsonui/SchemaSelectDialog.h"
#include "bsonui/SelectDialog.h"
#include "bsonui/TableEditWindow.h"
#include "bsonui/QueryWidget.h"
#include "bsonio/json2cfg.h"

#include "thermofun/DBClient/Traversal.h"
#include "thermofun/ReadFiles.h"

using namespace bsonio;

//  Connect all actions
void ThermoFunWidget::setActions()
{
    connect( ui->typeBox, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(typeChanged(const QString&)));

    // File
    connect( ui->actionNew_Reset_Task_Definition, SIGNAL( triggered()), this, SLOT(CmResetThermoFunData()));
    connect( ui->actionE_xit, SIGNAL( triggered()), this, SLOT(close()));
    connect( ui->actionExport_CfgFile, SIGNAL( triggered()), this, SLOT(CmExportCFG()));
    connect( ui->actionImport_CfgFile, SIGNAL( triggered()), this, SLOT(CmImportCFG()));

    // Edit
    connect(ui->actionCopy_Field_Path, SIGNAL(triggered()), fieldTable, SLOT(CopyFieldPath()));
    connect(ui->actionCopy_Field, SIGNAL(triggered()), fieldTable, SLOT(CopyField()));
    connect(ui->actionRealloc_TP, SIGNAL(triggered()), this, SLOT(CmReallocTP()));
    connect(ui->actionReset_TP, SIGNAL(triggered()), this, SLOT(CmResetTP()));
    connect(ui->actionChange_Property_list, SIGNAL(triggered()), this, SLOT(CmResetProperty()));

    // Help
    connect( ui->action_Help_About, SIGNAL( triggered()), this, SLOT(CmHelpAbout()));
    connect( ui->actionContents, SIGNAL( triggered()), this, SLOT(CmHelpContens()));
    connect( ui->actionAuthors, SIGNAL( triggered()), this, SLOT(CmHelpAuthors()));
    connect( ui->actionLicense, SIGNAL( triggered()), this, SLOT(CmHelpLicense()));

    // View
    connect( ui->action_Show_comments, SIGNAL( changed()), this, SLOT(CmShowComments()));
    connect( ui->action_Display_enums, SIGNAL( changed()), this, SLOT(CmDisplayEnums()));
    connect( ui->action_Edit_id, SIGNAL( changed()), this, SLOT(CmEditID()));
    connect( ui->actionKeep_Data_Fields_Expanded, SIGNAL( changed()), this, SLOT(CmEditExpanded()));
    updateViewMenu();

    // Record
    connect( ui->actionNext_Record, SIGNAL( triggered()), pTable, SLOT(CmNext()));
    connect( ui->actionPrevious_Record, SIGNAL( triggered()), pTable, SLOT(CmPrevious()));
    connect( ui->actionSearch_Results, SIGNAL( triggered()), this, SLOT(CmDisplaySearchResult()));
    connect( ui->actionSearch, SIGNAL( triggered()), this, SLOT(CmSearchQuery()));

    //TCorpt data
    connect( ui->pName, SIGNAL( textEdited(const QString&)), this, SLOT(nameChanged(const QString&)));
    connect( ui->pComment, SIGNAL( textEdited(const QString&)), this, SLOT(commentChanged(const QString&)));
    connect( ui->pTVal, SIGNAL( valueChanged(double)), this, SLOT(TChanged(double)));
    connect( ui->pTunit, SIGNAL( currentIndexChanged(const QString&)), this, SLOT(TUnitsChanged(const QString&)));
    connect( ui->pPVal, SIGNAL( valueChanged(double)), this, SLOT(PChanged(double)));
    connect( ui->pPunit, SIGNAL( currentIndexChanged(const QString&)), this, SLOT(PUnitsChanged(const QString&)));
    connect( ui->pPrecision, SIGNAL( valueChanged(int)), this, SLOT(pPChanged(int)));
    connect( ui->tPrecision, SIGNAL( valueChanged(int)), this, SLOT(tPChanged(int)));

    //Calc
    connect( ui->actionCalculate_Properties, SIGNAL( triggered()), this, SLOT(CmCalcMTPARM()));
    connect( ui->actionCalculate_TP, SIGNAL( triggered()), this, SLOT(CmCalcRTParm()));
    connect( ui->actionShow_Results, SIGNAL( triggered()), this, SLOT(CmShowResult()));


    QLineEdit* pLineTask = new QLineEdit( ui->nameToolBar );
    pLineTask->setEnabled( true );
    pLineTask->setFocusPolicy( Qt::ClickFocus );
    pLineTask->setReadOnly( true );
    pLineTask->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    QString title =  curSchemaName.c_str();
    pLineTask->setText(title);
    ui->nameToolBar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    ui->nameToolBar->addWidget( pLineTask ); // setStretchableWidget( pLine );

}

// Menu commands -----------------------------------------------------------

/// Set default bson record
void ThermoFunWidget::CmNew()
{
  try{
          bson_destroy( &curRecord );
          bson_init( &curRecord );
          bson_finish( &curRecord );
          resetBson("");
    }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


void ThermoFunWidget::CmDisplaySearchResult()
{
  if( dbgraph.get() == 0 )
       return;
  try
  {
     // define new dialog
     if(!queryResultWindow)
     {
         queryResultWindow = new TableEditWidget("Query Result window",
                         dataTable, TMatrixTable::tbShow );
     }
     else
     {
        queryResultWindow->updateTable();
        queryResultWindow->raise();
     }
     queryResultWindow->show();
  }
  catch(std::exception& e)
  {
        QMessageBox::critical( this, "CmDisplaySearchResult", e.what() );
  }
}

void ThermoFunWidget::CmSearchQuery()
{
  if( dbgraph.get() == 0 )
       return;

  try
  {
      // define new dialog
      if(!queryWindow)
      {
          queryWindow = new QueryWidget( "BSONUI Query Widget",
              schema, _shemaNames, dbgraph->getQuery(), this, this );

      }
      else
      {
         queryWindow->Update( dbgraph->getQuery() );
         queryWindow->raise();
      }
      queryWindow->show();

  }
  catch(std::exception& e)
  {
//      dbclient->SetQueryFields( oldkeys );
//      dbclient->SetQueryJson( oldquery );
      QMessageBox::critical( this, "CmSearchQuery", e.what() );
  }
}

void ThermoFunWidget::setQuery( QueryWidget* queryW  )
{
  if( dbgraph.get() == 0 )
       return;

  DBQueryDef oldquery = dbgraph->getQuery();
  try
  {
      DBQueryDef query = queryW->getQueryDef();
      // reset internal query data
      if( query.getEJDBQuery() != oldquery.getEJDBQuery() )
      {
        isDefaultQuery = true;
        _data.query = query.getEJDBQuery();

      }
      updateQuery( query  );
  }
  catch(std::exception& e)
  {
      dbgraph->runQuery(oldquery);
      QMessageBox::critical( this, "CmSearchQuery", e.what() );
  }
}

void ThermoFunWidget::updateQuery( const DBQueryDef& query  )
{
  if( dbgraph.get() == 0 )
       return;
  dbgraph->runQuery(query);
  // update search tables
  ui->edgeQuery->setText(query.getEJDBQuery().c_str());
  tableModel->resetMatrixData();
  if( queryResultWindow )
    queryResultWindow->updateTable();
}



// new commands -------------------------------------------------------------------

void ThermoFunWidget::CmReallocTP()
{
  try{
        bool ok = 0;
        uint size = QInputDialog::getInt( this, "Please, select new TP pairs array size",
                 "Array size ", _data.tppairs.size(), 0, 999, 1, &ok );
        if(!ok) // cancel command
            return;

        _data.tppairs.clear();
        for (uint i = 0; i<size; i++ )
        {
            _data.tppairs.push_back({0,0});
        }
        _TPlistModel->resetMatrixData();
    }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


void ThermoFunWidget::CmResetTP()
{
  try{
        // define new preferences
        TPSetDialog dlg( this);
         if( !dlg.exec() )
             return;

         ui->pTunit->setCurrentText(dlg.getTUnits());
         ui->pPunit->setCurrentText(dlg.getPUnits());
        _data.tppairs = dlg.getTPpairs();
        _TPlistModel->resetMatrixData();
    }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


void ThermoFunWidget::CmResetProperty()
{
 try
  {   
    std::size_t pos = std::string("Vertex").length();
    std::string schemaName = "ThermoProperties" + curSchemaName.substr (pos);
    SchemaSelectDialog dlg(this, "Please, mark property fields",
        schema, schemaName, _data.properties, _data.properties);
    if( !dlg.exec() )
       return;

    // store previous settings
    for (unsigned i = 0; i<_data.properties.size(); i++)
    {
        _data.mapUnits[_data.properties[i]] = _data.propertyUnits[i];
        _data.mapPrecision[_data.properties[i]] = _data.propertyPrecision[i];
    }

    _data.properties = dlg.allSelected();
    _data.propertyUnits.resize(_data.properties.size());
    _data.propertyPrecision.resize(_data.properties.size());
    for (unsigned i = 0; i<_data.properties.size(); i++)
    {
        _data.propertyUnits[i] = _data.mapUnits[_data.properties[i]];
        _data.propertyPrecision[i] = _data.mapPrecision[_data.properties[i]];
    }
     _PropertyModel->resetMatrixData();
  }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }

}

void ThermoFunWidget::CmResetThermoFunData()
{
    ThermoFunData dt;

    _data = dt;

    // define ThermoFun data
    _TPContainer = new TPContainer( "T", { "T", "P" }, _data.tppairs );
    _TPlistTable  = new TMatrixTable( ui->outWidget );
    TMatrixDelegate* deleg = new TMatrixDelegate();
    _TPlistTable->setItemDelegate(deleg);
    _TPlistModel = new TMatrixModel( _TPContainer, this );
    _TPlistTable->setModel(_TPlistModel);
    _TPlistTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch/*Interactive*/ );
    ui->gridLayout_3->addWidget(_TPlistTable, 1, 0, 1, 1);

    _PropertyContainer = new TPropertyContainer( "Property", _data.properties, _data.propertyUnits, _data.propertyPrecision );
    _PropertyTable  = new TMatrixTable( ui->inWidget );
     deleg = new TMatrixDelegate();
    _PropertyTable->setItemDelegate(deleg);
    _PropertyModel = new TMatrixModel( _PropertyContainer, this );
    _PropertyTable->setModel(_PropertyModel);
    _PropertyTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch/*Interactive*/ );
    ui->gridLayout_2->addWidget(_PropertyTable, 6, 0, 1, 7);
    ui->pName->setText(_data.name.c_str());
    ui->pComment->setText(_data.comment.c_str());
    ui->pTVal->setValue(_data.T);
    ui->pPVal->setValue(_data.P);
    ui->pTunit->setCurrentText( _data.unitsT.c_str());
    ui->pPunit->setCurrentText(_data.unitsP.c_str());
    ui->pPrecision->setValue(_data.pPrecision);
    ui->tPrecision->setValue(_data.tPrecision);

    ui->calcStatus->setText(_data.calcStatus.c_str());

    ui->actionShow_Results->setEnabled(false);
}

/// Read bson record from json file fileName
void ThermoFunWidget::CmImportCFG()
{
  try{
        string fileName;
        if(  ChooseFileOpen( this, fileName,
                     "Please, select Structured Data file", "*.json"  ))
        { //_data.readfromCFG(fileName);
          FJson file( fileName);
          bson obj;
          file.LoadBson( &obj );
          _data.fromBson(obj.data);
          bson_destroy(&obj);
          resetThermoFunData();
        }
    }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


/// Write current task to configuration file file
void ThermoFunWidget::CmExportCFG()
{
   try {
         string fileName = _data.name+".json";
         if(  ChooseFileSave( this, fileName,
                     "Please, select file to write the data", "*.json", fileName  ))
         {
             //_data.savetoCFG(fileName);
             FJson file( fileName);
             bson obj;
             bson_init(&obj);
             _data.toBson(&obj);
             bson_finish(&obj);
             file.SaveBson( obj.data );
             bson_destroy(&obj);
         }
    }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

std::map<std::string, int> ThermoFunWidget::getReactantsCoeffMap(std::string _id)
{
    std::map<std::string, int> map;
    string kbuf;
    int stoi_coeff;
    bson record;
    string qrJson = "{'_type': 'edge', '_label': 'takes', '_inV': '";
    qrJson += _id;
    qrJson += "' }";

    vector<string> _queryFields = { "_outV", "properties.stoi_coeff"};
    vector<string> _resultDataEdge, _resultDataSubst;
    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "properties.stoi_coeff", kbuf );
        stoi_coeff = atoi(kbuf.c_str());

        bsonio::bson_to_key( record.data, "_outV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataSubst);

        if (_resultDataSubst.size()>0)
        {
            jsonToBson(&record, _resultDataSubst[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
        }

        map.insert(std::pair<std::string,int>(kbuf,stoi_coeff));
    }

    return map;
}

std::string ThermoFunWidget::getDefinesReactionSymbol(std::string _idSubst)
{
    string kbuf = "";
    bson record;
    string qrJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
    qrJson += _idSubst;
    qrJson += "' }";

    vector<string> _queryFields = { "_outV", "_label"};
    vector<string> _resultDataEdge, _resultDataReac;
    dbgraph->runQuery( qrJson,  _queryFields, _resultDataEdge );

    for(uint i = 0; i < _resultDataEdge.size(); i++)
    {
        jsonToBson(&record, _resultDataEdge[i]);
        bsonio::bson_to_key( record.data, "_outV", kbuf );
        qrJson = "{ \"_id\" : \""+kbuf+ "\"}";
        dbgraph->runQuery(qrJson, {"_id", "_label", "properties.symbol"}, _resultDataReac);

        if (_resultDataReac.size()>0)
        {
            jsonToBson(&record, _resultDataReac[0]);
            bsonio::bson_to_key( record.data, "properties.symbol", kbuf );
        }
    }
    return kbuf;
}

void ThermoFunWidget::CmCalcMTPARM()
{
    if( dbgraph.get() == 0 )
          return;
   vector<bson> selectedList;
   int number_selected_solvent = 0;

   DBQueryDef oldquery( dbgraph->getQuery());

   try {
       // Select keys to send to ThermoFun
       bool isSolvent = false;
       string key, valDB, _idSubst;
       bson record;
       vector<string> aKeyList;
       vector<vector<string>> aValList;
       vector<int> selNdx, selNdx_;
       vector<string> substancesSymbols, substancesClass;
       dbgraph->GetKeyValueList( aKeyList, aValList );
       string solventSymbol;
       typedef ThermoFun::MapIdBson::iterator it_;

       // The set of all aqueous species in the database
       ThermoFun::SubstancesMap substances_map;
       // The set of all gaseous species in the database
       ThermoFun::ReactionsMap reactions_map;

       if( aKeyList.empty() )
           return;

       SelectDialog selDlg( this, "Please, select one or more records", aValList, selNdx );
       if( !selDlg.exec() )
           return;
       selNdx =  selDlg.allSelected();
       selectedList.resize(selNdx.size());
       selNdx_= selNdx;
       substancesSymbols.resize(selNdx.size()); substancesClass.resize(selNdx.size());

      for( uint ii=0; ii<selNdx.size(); ii++ )
      {
        string key = aKeyList[selNdx[ii]];
        dbgraph->GetRecord( key.c_str() );
        string valDB = dbgraph->GetJson();
        jsonToBson( &selectedList[ii], valDB );
        bsonio::bson_to_key( selectedList[ii].data, ThermoFun::substSymbol, substancesSymbols[ii]);
        bsonio::bson_to_key( selectedList[ii].data, ThermoFun::substClass, substancesClass[ii]);
      }

      // check for solvent
      for (uint ii=0; ii<substancesClass.size(); ii++)
      {
          if (stoi(substancesClass[ii]) == ThermoFun::SubstanceClass::type::AQSOLVENT)
          {
              isSolvent = true;
              number_selected_solvent++;
              solventSymbol = substancesSymbols[ii];
          }
      }

      if (!isSolvent || number_selected_solvent>1)
      {
         // query the solvent
              string qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.class_\" : 3}]}";
              DBQueryDef newquery = dbgraph->getQuery();
              newquery.setEJDBQuery(qrJson);
              updateQuery( newquery  );

              dbgraph->GetKeyValueList( aKeyList, aValList );
oneSolvent:
          SelectDialog selDlg2( this, "Please, select one solvent (e.g. H2O@) for solute properties calculation",
                                aValList, selNdx );
          if( !selDlg2.exec() )
              return;
          selNdx = selDlg2.allSelected();

          if (selNdx.size() != 1)
              goto oneSolvent;
          bson solvent; string symbol, class_;

          string key = aKeyList[selNdx[0]];
          dbgraph->GetRecord( key.c_str() );
          string valDB = dbgraph->GetJson();
          jsonToBson( &solvent, valDB );

          bsonio::bson_to_key( solvent.data, ThermoFun::substClass, class_);
          bsonio::bson_to_key( solvent.data, ThermoFun::substSymbol, symbol);

          bool wasSelected = false;

          std::vector<string>::iterator it = std::find(substancesSymbols.begin(), substancesSymbols.end(), symbol);

          if (it != substancesSymbols.end())
              wasSelected = true;
          else
              wasSelected = false;

          if (!wasSelected)
          {
              selectedList.push_back(solvent);

              ThermoFun::Substance solv = ThermoFun::parseSubstance(solvent.data);

              substances_map[symbol] = solv;
              selNdx_.push_back(selNdx[0]);

          }

          solventSymbol = symbol;

      }

      // reset to the old list of substances
      updateQuery( oldquery  );
      dbgraph->GetKeyValueList( aKeyList, aValList );

      ThermoFun::Traversal tr(dbgraph);

      ThermoFun::MapIdBson map_ = tr.getLinkedSelectedData(selNdx_, aKeyList);

      // get substances and the reaction symbol if necessary
      for(it_ iterator = map_.begin(); iterator != map_.end(); iterator++)
      {
          if (iterator->second == "substance")
          {
              resetDBClient("VertexSubstance");

              key = iterator->first +":";
              dbgraph->GetRecord( key.c_str() );
              valDB = dbgraph->GetJson();
              jsonToBson( &record, valDB );
              bsonio::bson_to_key( record.data, "_id", _idSubst );

              ThermoFun::Substance substance = ThermoFun::parseSubstance(record.data);

              if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
                  substances_map[substance.symbol()] = substance;
              } else {
                  // ERROR substance with the same symbol found!
              }

              // get reaction symbol which define substance with _idSubst
              if (getDefinesReactionSymbol(_idSubst) != "")
              {
                  substance.setReactionSymbol(getDefinesReactionSymbol(_idSubst));
                  substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
              }

              substances_map[substance.symbol()] = substance;
          } else
              if (iterator->second == "reaction")
              {
                  resetDBClient("VertexReaction");

                  key = iterator->first +":";
                  dbgraph->GetRecord( key.c_str() );
                  valDB = dbgraph->GetJson();
                  jsonToBson( &record, valDB );
                  bsonio::bson_to_key( record.data, "_id", key );

                  ThermoFun::Reaction reaction = ThermoFun::parseReaction(record.data);

                  if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
                      reactions_map[reaction.symbol()] = reaction;
                  } else {
                      // ERROR reaction with the same symbol found!
                  }

                  // get reactants by following reaction incoming takes edge
                  reaction.setReactants(getReactantsCoeffMap(key));

                  reactions_map[reaction.symbol()] = reaction;
              }

      }
      resetDBClient("VertexSubstance");

      ThermoFun::Database tdb;
      tdb.addMapReactions(reactions_map);
      tdb.addMapSubstances(substances_map);

      ThermoFun::Interface tpCalc (tdb);
      tpCalc.setSolventSymbolForAqSubst(solventSymbol);

      ThermoFun::OutputSettings op;
      if (ui->FormatBox->isChecked())
      {
          op.isFixed = true;
          tpCalc.setOutputSettings(op);
      }

      std::map<const std::string, int> precision = ThermoFun::defaultPropertyDigits;
      for (uint jj = 0; jj <_data.properties.size(); jj++)
      {
          precision.at(_data.properties[jj]) = _data.propertyPrecision[jj];
      }

      precision.at("temperature") = _data.tPrecision;
      precision.at("pressure") = _data.pPrecision;

      tpCalc.addDigits(precision);

      struct timeval start, end;
      gettimeofday(&start, NULL);

      tpCalc.calculateProperties(substancesSymbols, _data.properties, _data.tppairs).toCSV(op.fileNameSubst);

      gettimeofday(&end, NULL);
      double delta_calc = ((end.tv_sec  - start.tv_sec) * 1000000u +
                           end.tv_usec - start.tv_usec) / 1.e6;

      string status = "Calculations finished ("+ to_string(delta_calc) + "s). View results.";

      ui->calcStatus->setText(status.c_str());
      ui->actionShow_Results->setEnabled(true);

   }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
       ui->calcStatus->setText(e.what());
   }
   catch(std::exception& e)
   {
       QMessageBox::critical( this, "std::exception", e.what() );
       ui->calcStatus->setText(e.what());
   }

}

void ThermoFunWidget::CmCalcRTParm()
{
   try {
        cout << "CmCalcRTParm" << endl;

    }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


//Result
void ThermoFunWidget::CmShowResult()
{
   try {
        // define new dialog
        ThermoFun::OutputSettings op;
        string fileName = op.fileNameSubst;
        if(!_csvWin)
        {
            _csvWin = new TableEditWidget("CSV editor ", fileName,
              TMatrixTable::tbEdit|TMatrixTable::tbGraph/*|TMatrixTable::tbSort*/ );
        }
        else
        {
           _csvWin->openNewCSV(fileName);
           _csvWin->raise();
        }
        _csvWin->show();
    }
   catch(bsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}



// end of ThermoFunMenu.cpp
