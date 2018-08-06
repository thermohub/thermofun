//  This is ThermoFun library+API (https://bitbucket.org/gems4/ThermoFun)
//
/// \file ThermoFunWidget.cpp
/// ThermoFunWidget - Widget to work with ThermoFun data
//
// JSONUI is a C++ Qt5-based widget library and API aimed at implementing
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
// JSONUI depends on the following open-source software products:
// jsonio (https://bitbucket.org/gems4/jsonio); Qt5 (https://qt.io);
// Qwtplot (http://qwt.sourceforge.net).

#include <iostream>
#include <QMessageBox>
#include <QKeyEvent>
#include <QInputDialog>
#include <sys/time.h>
#include "jsonio/json2cfg.h"
#include "jsonio/jsondomfree.h"
#include "jsonui/SchemaSelectDialog.h"
#include "jsonui/TableEditWindow.h"
#include "jsonui/QueryWidget.h"
#include "ThermoFunWidgetNew.h"
#include "ui_ThermoFunWidget.h"
#include "ThermoFunPrivateNew.h"
#include "TPSetDialog.h"
#include "SelectThermoDataDialog.h"
using namespace jsonio;
using namespace jsonui;


ThermoFunWidgetNew::ThermoFunWidgetNew( QWidget *parent) :
    JSONUIBase( "", parent),
    ui(new Ui::ThermoFunWidget),
    /*queryWindow(0), queryResultWindow(0),*/ _csvWin(0)
{
    // set up widget data
    ui->setupUi(this);
    ui->keySplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setCollapsible(0, false);
    ui->mainSplitter->setCollapsible(1, false);
    //define schema checkbox
    _shemaNames.push_back("VertexSubstance");
    _shemaNames.push_back("VertexReaction");
    _typeNames.push_back("Substances");
    _typeNames.push_back("Reactions");
    for( uint ii=0; ii<_typeNames.size(); ii++ )
      ui->typeBox->addItem(_typeNames[ii].c_str());

    setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed
    QString title = qApp->applicationName()+" application for tabulating standard state thermodynamic properties";
    setWindowTitle(title);

    // init internal data
    pdata.reset( new ThermoFunPrivateNew(this) );

    // define menu
    setActions();
    initSourceTDB();
    resetThermoFunData(pdata->data());

    show();
    CmSelectThermoDataSet();
}

ThermoFunWidgetNew::~ThermoFunWidgetNew()
{
    // free keys query data
    /*if(queryResultWindow)
    {
       queryResultWindow->close();
       delete queryResultWindow;
    }
    if(queryWindow)
    {
       queryWindow->close();
       delete queryWindow;
    }*/
    if( _csvWin )
    {
       _csvWin->close();
       delete _csvWin;
    }
    delete ui;
    cout << "~ThermoFunWidget" << endl;
}

void ThermoFunWidgetNew::closeEvent(QCloseEvent* e)
{
    /*if( queryResultWindow )
     queryResultWindow->close();

    if( queryWindow )
     queryWindow->close();
    */
    if( _csvWin )
     _csvWin->close();

    if( !onCloseEvent(this) )
           e->ignore();
       else
           QWidget::closeEvent(e);

}

void ThermoFunWidgetNew::initSourceTDB()
{
    ThriftEnumDef* enumdef =  ioSettings().Schema()->getEnum("SourceTDB" );
    if(enumdef != nullptr )
    {
        vector<string> lst = enumdef->getNamesList();
        for( uint ii=0; ii<lst.size(); ii++ )
        {
            string itname =lst[ii];
            ui->pSourceTDB->addItem(tr(itname.c_str()), enumdef->getId(itname) );
            ui->pSourceTDB->setItemData(ii, enumdef->getDoc(itname).c_str(), Qt::ToolTipRole);
        }
    }
    ui->pSourceTDB->setCurrentText("CODATA");
}

//  Connect all actions
void ThermoFunWidgetNew::setActions()
{
    connect( ui->typeBox, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(typeChanged(const QString&)));

    // File
    connect( ui->actionNew_Reset_Task_Definition, SIGNAL( triggered()), this, SLOT(CmResetThermoFunData()));
    connect( ui->actionE_xit, SIGNAL( triggered()), this, SLOT(close()));
    connect( ui->actionExport_CfgFile, SIGNAL( triggered()), this, SLOT(CmExportCFG()));
    connect( ui->actionImport_CfgFile, SIGNAL( triggered()), this, SLOT(CmImportCFG()));

    // Edit
    connect(ui->actionRealloc_TP, SIGNAL(triggered()), this, SLOT(CmReallocTP()));
    connect(ui->actionReset_TP, SIGNAL(triggered()), this, SLOT(CmResetTP()));
    connect(ui->actionChange_Property_list, SIGNAL(triggered()), this, SLOT(CmResetProperty()));
    connect(ui->actionSelect_ThermoDataSet, SIGNAL(triggered()), this, SLOT(CmSelectThermoDataSet()));
    connect(ui->actionSelect_Elements, SIGNAL(triggered()), this, SLOT(CmSelectSourceTDBs()));
    connect(ui->action_Set_Elemets_to_reactions, SIGNAL(triggered()), this, SLOT(CmSetElementsReactions()));
    connect(ui->action_Set_Elemets_to_reactionsets, SIGNAL(triggered()), this, SLOT(CmSetElementsReactionSets()));

    // Help
    connect( ui->action_Help_About, SIGNAL( triggered()), this, SLOT(CmHelpAbout()));
    connect( ui->actionContents, SIGNAL( triggered()), this, SLOT(CmHelpContens()));
    connect( ui->actionAuthors, SIGNAL( triggered()), this, SLOT(CmHelpAuthors()));
    connect( ui->actionLicense, SIGNAL( triggered()), this, SLOT(CmHelpLicense()));

    // Record
    //?? connect( ui->actionSearch_Results, SIGNAL( triggered()), this, SLOT(CmDisplaySearchResult()));
    //?? connect( ui->actionSearch, SIGNAL( triggered()), this, SLOT(CmSearchQuery()));

    //TCorpt data
    connect( ui->pName, SIGNAL( textEdited(const QString&)), pdata.get(), SLOT(nameChanged(const QString&)));
    connect( ui->pComment, SIGNAL( textEdited(const QString&)), pdata.get(), SLOT(commentChanged(const QString&)));
    connect( ui->pTunit, SIGNAL( currentIndexChanged(const QString&)), pdata.get(), SLOT(TUnitsChanged(const QString&)));
    connect( ui->pPunit, SIGNAL( currentIndexChanged(const QString&)), pdata.get(), SLOT(PUnitsChanged(const QString&)));
    connect( ui->pPrecision, SIGNAL( valueChanged(int)), pdata.get(), SLOT(pPChanged(int)));
    connect( ui->tPrecision, SIGNAL( valueChanged(int)), pdata.get(), SLOT(tPChanged(int)));

    //Calc
    connect( ui->actionCalculate_Properties, SIGNAL( triggered()), this, SLOT(CmCalcMTPARM()));
    connect( ui->actionCalculate_TP, SIGNAL( triggered()), this, SLOT(CmCalcRTParm()));
    connect( ui->actionShow_Results, SIGNAL( triggered()), this, SLOT(CmShowResult()));

    // Preferences
    connect( ui->actionCalculate_Substances_Records_from_Dependent_Reactions, SIGNAL( toggled(bool)), this, SLOT(CmCalcSubstFromReact(bool)));
    connect( ui->actionCalculate_Reactions_Records_from_Reactants           , SIGNAL( toggled(bool)), this, SLOT(CmCalcReactFromSubst(bool)));

    ui->actionCalculate_Reactions_Records_from_Reactants->setChecked(false);
    ui->actionCalculate_Reactions_Records_from_Reactants->setEnabled(false);

    ui->actionReset_TP->setEnabled(false);
    ui->actionRealloc_TP->setEnabled(false);
    ui->actionChange_Property_list->setEnabled(false);
    ui->actionCalculate_Properties->setEnabled(false);
    ui->actionShow_Results->setEnabled(false);


    //QLineEdit* pLineTask = new QLineEdit( ui->nameToolBar );
    //pLineTask->setEnabled( true );
    //pLineTask->setFocusPolicy( Qt::ClickFocus );
    //pLineTask->setReadOnly( true );
    //pLineTask->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    //QString title =  curSchemaName.c_str();
    //pLineTask->setText(title);
    ui->nameToolBar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    //ui->nameToolBar->addWidget( pLineTask ); // setStretchableWidget( pLine );
}


//-----------------------------------------------------

void ThermoFunWidgetNew::resetTypeBox( const QString& text )
{
    disconnect( ui->typeBox, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(typeChanged(const QString&)));
    ui->typeBox->setCurrentText(text);
    connect( ui->typeBox, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(typeChanged(const QString&)));
}

// Reset ThermoFun data
void ThermoFunWidgetNew::resetThermoFunData( const ThermoFunData& newdata )
{
    pdata->newThermoFunData(newdata);

    // update view
    ui->pName->setText(newdata.name.c_str());
    ui->pComment->setText(newdata.comment.c_str());
    ui->pTunit->setCurrentText( newdata.unitsT.c_str());
    ui->pPunit->setCurrentText(newdata.unitsP.c_str());
    ui->pPrecision->setValue(newdata.pPrecision);
    ui->tPrecision->setValue(newdata.tPrecision);
    ui->calcStatus->setText(newdata.calcStatus.c_str());
    //?? ui->edgeQuery->setText(newdata.query.getQueryString().c_str());
    ui->actionShow_Results->setEnabled(false);
    ui->actionCalculate_Properties->setEnabled(false);
    ui->actionChange_Property_list->setEnabled(false);
    ui->typeBox->setEnabled(false);
    ui->actionReset_TP->setEnabled(false);
    ui->pSolventSymbol->clear();
    resetTypeBox( newdata.schemaName.c_str() );
}


// Menu commands -----------------------------------------------------------

void ThermoFunWidgetNew::CmResetThermoFunData()
{
    ThermoFunData dt;
    dt.resetSchemaName( pdata->data().schemaName );
    resetThermoFunData(dt);
}


// Change current Vertex
void ThermoFunWidgetNew::typeChanged(const QString& text)
{
  try {
        string newname = text.toStdString();
        if (newname == "Substances")
        {
            pdata->typeChanged( "VertexSubstance" );
            calcReactFromSubst = false;
            ui->actionCalculate_Substances_Records_from_Dependent_Reactions->setEnabled(true);
            ui->actionCalculate_Reactions_Records_from_Reactants->setChecked(false);
            ui->actionCalculate_Reactions_Records_from_Reactants->setEnabled(false);
        }
        if (newname == "Reactions")
        {
            pdata->typeChanged( "VertexReaction" );
            calcSubstFromReact = false;
            ui->actionCalculate_Substances_Records_from_Dependent_Reactions->setChecked(false);
            ui->actionCalculate_Substances_Records_from_Dependent_Reactions->setEnabled(false);
            ui->actionCalculate_Reactions_Records_from_Reactants->setEnabled(true);
        }
    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


void ThermoFunWidgetNew::CmSelectThermoDataSet()
{
  try {
       SelectThermoDataDialog dlg( pdata->data().idThermoDataSet, pdata->data().elements, pdata->dbclient, this);

      if( dlg.exec() )
      {
        vector<ThermoFun::ElementKey> elKeys;
        dlg.allSelected( elKeys );
        /*pdata->updateElements(dlg.sourceTDB(), elKeys, dlg.idReactionSet() );
        update();

        auto substDataNdx = pdata->getSubstDataIndex();
        solventsIds.clear();
        ui->pSolventSymbol->clear();

        ValuesTable solvents = pdata->querySolvents();

        if (solvents.size() == 0)
        {
            ui->pSolventSymbol->addItem("No solvent!", 0);
        }

        for (uint ii = 0; ii<solvents.size(); ii++)
        {
            string sSymbol = solvents[ii][substDataNdx["symbol"]];
            solventsIds.push_back(solvents[ii][substDataNdx["_id"]]);
            ui->pSolventSymbol->addItem(sSymbol.c_str(), ii);
        }*/

        ui->actionReset_TP->setEnabled(true);
        ui->actionRealloc_TP->setEnabled(true);
        ui->actionChange_Property_list->setEnabled(true);
        ui->actionCalculate_Properties->setEnabled(true);
        ui->typeBox->setEnabled(true);
        ui->calcStatus->setText("Set temperature and pressure points, set properties to calculate, and click calculate."); // status
      }
    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

void ThermoFunWidgetNew::CmReallocTP()
{
  try{
        bool ok = 0;
        uint size = QInputDialog::getInt( this, "Please, select new TP pairs array size",
                 "Array size ", pdata->data().tppairs.size(), 0, 999, 1, &ok );
        if(ok)
          pdata->reallocTP( size );
    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

void ThermoFunWidgetNew::CmResetTP()
{
  try{
        TPSetDialog dlg( this);
        if( dlg.exec() )
           pdata->updateTP( dlg.getTUnits().toStdString(),
                  dlg.getPUnits().toStdString(), dlg.getTPpairs() );
    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

void ThermoFunWidgetNew::CmResetProperty()
{
 try
  {
    std::string schemaName = pdata->getThermoPropertiesName();
    SchemaSelectDialog dlg(this, "Please, mark property fields",
         schemaName, pdata->data().properties);
    if( dlg.exec() )
      pdata->updateProperty( dlg.allSelected() );
  }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }

}

/// Read bson record from json file fileName
void ThermoFunWidgetNew::CmImportCFG()
{
  try{
        string fileName;
        if(  ChooseFileOpen( this, fileName,
                     "Please, select Structured Data file", "*.json"  ))
        { //_data.readfromCFG(fileName);
          // read data to dom
          shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
          FJson file( fileName);
          file.LoadJson( domdata.get() );
          ThermoFunData thdata;
          thdata.fromJsonNode(domdata.get());
          resetThermoFunData(thdata);
        }
    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

/// Write current task to configuration file file
void ThermoFunWidgetNew::CmExportCFG()
{
   try {
         string fileName = pdata->data().name+".json";
         if(  ChooseFileSave( this, fileName,
                     "Please, select file to write the data", "*.json", fileName  ))
         {
             //_data.savetoCFG(fileName);
             FJson file( fileName);
             shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
             pdata->data().toJsonNode(domdata.get());
             file.SaveJson( domdata.get() );
         }
    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


void ThermoFunWidgetNew::CmCalcMTPARM()
{
    try {
            // select components
            const ValuesTable& values= pdata->getValues( pdata->isSubstances() );
            vector<int> selNdx;
            SelectDialog selDlg( this, "Please, select one or more records", values, selNdx );
            if( !selDlg.exec() )
                return;
            selNdx =  selDlg.allSelected();

//            MapSymbolMapLevelReaction   levelDefinesReaction  = pdata->recordsMapLevelDefinesReaction(/*3, 0*/);

            struct timeval start;
            gettimeofday(&start, NULL);
            // load data
            vector<string> substKeys, reactKeys;
            vector<string> substancesSymbols, substancesClass, reactionsSymbols;

            if( pdata->isSubstances() )
                pdata->loadSubstData( selNdx, substKeys, substancesSymbols, substancesClass );
            else
                pdata->loadReactData( selNdx, reactKeys, reactionsSymbols );

            vector<string> linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses, linkedSubstIds;
            pdata->retrieveConnectedDataSymbols(substKeys, reactKeys, linkedSubstSymbols, linkedReactSymbols, linkedSubstClasses, linkedSubstIds);

            if (calcReactFromSubst && calcSubstFromReact) // check - reaction with substance dependent on another reaction
            {
                MapSymbolMapLevelReaction   levelDefinesReaction  = pdata->recordsMapLevelDefinesReaction(linkedSubstIds, linkedSubstSymbols);
                for (auto substSymbol : linkedSubstSymbols)
                {
                    MapLevelReaction    levelReaction   = levelDefinesReaction[substSymbol];
                    if (levelReaction.size() == 0)
                        continue;
                    if (levelReaction.size() == 1)
                    {
                        pdata->setSubstanceLevel(substSymbol, levelReaction.begin()->first);
                    }
                    if (levelReaction.size() > 1)
                    {
                        ValuesTable values_;
                        vector<string> levels;
                        for (auto react : levelReaction)
                        {
                            vector<string> symbolEq;
                            symbolEq.push_back(react.second.symbol());
                            symbolEq.push_back(react.second.equation());
                            levels.push_back(react.first);
                            values_.push_back(symbolEq);
                        }

                        SelectDialog selDlg_lvl( this, ("Please, select the reaction which defines substance: "+substSymbol).c_str(), values_ );
                        if( !selDlg_lvl.exec() )
                            return;
                        int solvNdx =  selDlg_lvl.selIndex();

                        pdata->setSubstanceLevel(substSymbol, levels[solvNdx]);
                    }
                }
            }

            // check for solvent
            string solventSymbol = ui->pSolventSymbol->currentText().toStdString();
            if (solventSymbol != "No solvent!")
            {
                auto sId = solventsIds[ui->pSolventSymbol->currentIndex()] /*+":"*/;
                if (std::find(substKeys.begin(), substKeys.end(), sId) == substKeys.end())
                    substKeys.push_back(sId);
            }

            // calculate task
            double delta_calc = pdata->calcData( substKeys, reactKeys,
               substancesSymbols,  reactionsSymbols, solventSymbol, ui->actionFixed_output_number_format->isChecked(), calcSubstFromReact, calcReactFromSubst, start );

           string status = "Calculation finished ("+ to_string(delta_calc) + "s). Click view results."; // status

            ui->calcStatus->setText(status.c_str());
            ui->actionShow_Results->setEnabled(true);

    }
    catch(jsonio_exeption& e)
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

void ThermoFunWidgetNew::CmCalcRTParm()
{
   try {
        cout << "CmCalcRTParm" << endl;

    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


//Result
void ThermoFunWidgetNew::CmShowResult()
{
   try {
        // define new dialog
        ThermoFun::OutputSettings op;
        string fileName = op.fileName;
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
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

//Preferences
void ThermoFunWidgetNew::CmCalcSubstFromReact(bool togled)
{
    calcSubstFromReact = togled;
}

void ThermoFunWidgetNew::CmCalcReactFromSubst(bool togled)
{
    calcReactFromSubst = togled;
}


/// temporary function to reset old reactions
void ThermoFunWidgetNew::CmSetElementsReactions()
{
   try {

     auto graphdb = pdata->dbclient.reactData().getDB();
     // Select keys to delete
     vector<string> aKeyList;
     vector<vector<string>> aValList;
     vector<int> selNdx;
     graphdb->GetKeyValueList( aKeyList, aValList );
     if( aKeyList.empty() )
       return;

     SelectDialog selDlg( this, "Please, select a record to update", aValList, selNdx );
      if( !selDlg.exec() )
          return;

     selNdx =  selDlg.allSelected();
     for( uint ii=0; ii<selNdx.size(); ii++ )
       pdata->resetElementsintoRecord( true, aKeyList[selNdx[ii]]);

   }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

/// temporary function to reset old reactionSets
void ThermoFunWidgetNew::CmSetElementsReactionSets()
{
   try {

     auto graphdb = pdata->dbclient.reactSetData().getDB();
     // Select keys to delete
     vector<string> aKeyList;
     vector<vector<string>> aValList;
     vector<int> selNdx;
     graphdb->GetKeyValueList( aKeyList, aValList );
     if( aKeyList.empty() )
       return;

     SelectDialog selDlg( this, "Please, select a record to update", aValList, selNdx );
      if( !selDlg.exec() )
          return;

     selNdx =  selDlg.allSelected();
     for( uint ii=0; ii<selNdx.size(); ii++ )
       pdata->resetElementsintoRecord(false, aKeyList[selNdx[ii]]);

   }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


/// Update after change DB locations
void ThermoFunWidgetNew::updtDB()
{
    /*queryWindow->close();
    queryResultWindow->close();*/
    pdata->updateDBClient();
    CmResetThermoFunData();
}


// old not used

/*void ThermoFunWidgetNew::CmDisplaySearchResult()
{
  try
  { }
  catch(std::exception& e)
  {
        QMessageBox::critical( this, "CmDisplaySearchResult", e.what() );
  }
}

void ThermoFunWidgetNew::CmSearchQuery()
{
  try
  {
      if(!queryWindow)
      {
          queryWindow = new QueryWidget( "JSONUI Query Widget",
            _shemaNames, pdata->getQuery(), this, this );
//          { curSchemaName }, pdata->tableModel->getQuery(), this, this );
      }
      else
      {
         queryWindow->Update( pdata->getQuery() );
         queryWindow->raise();
      }
      queryWindow->show();

  }
  catch(std::exception& e)
  {
      QMessageBox::critical( this, "CmSearchQuery", e.what() );
  }
}

void ThermoFunWidgetNew::setQuery( QueryWidget* queryW  )
{
  try
  {
      auto newquery = queryW->getQueryDef().getQueryCondition();
      pdata->updateQuery( newquery );
      ui->edgeQuery->setText(newquery.getQueryString().c_str());
      if( queryResultWindow )
        queryResultWindow->updateTable();
  }
  catch(std::exception& e)
  {
      QMessageBox::critical( this, "CmSearchQuery", e.what() );
  }
}


void ThermoFunWidgetNew::CmSelectElements()
{
  try {
       SelectElementsDialog dlg( pdata->dbclient, this);
      dlg.setData( pdata->data().sourcetdb, pdata->data().elements, pdata->data().idReactionSet );
      if( dlg.exec() )
      {
        vector<ThermoFun::ElementKey> elKeys;
        dlg.allSelected( elKeys );
        pdata->updateElements(dlg.sourceTDB(), elKeys, dlg.idReactionSet() );
        update();

        auto substDataNdx = pdata->getSubstDataIndex();
        solventsIds.clear();
        ui->pSolventSymbol->clear();

        ValuesTable solvents = pdata->querySolvents();

        if (solvents.size() == 0)
        {
            ui->pSolventSymbol->addItem("No solvent!", 0);
        }

        for (uint ii = 0; ii<solvents.size(); ii++)
        {
            string sSymbol = solvents[ii][substDataNdx["symbol"]];
            solventsIds.push_back(solvents[ii][substDataNdx["_id"]]);
            ui->pSolventSymbol->addItem(sSymbol.c_str(), ii);
        }

        ui->actionReset_TP->setEnabled(true);
        ui->actionRealloc_TP->setEnabled(true);
        ui->actionChange_Property_list->setEnabled(true);
        ui->actionCalculate_Properties->setEnabled(true);
        ui->typeBox->setEnabled(true);
        ui->calcStatus->setText("Set temperature and pressure points, set properties to calculate, and click calculate."); // status
      }

    }
   catch(jsonio_exeption& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}
*/

// end of ThermoFunMenu.cpp


