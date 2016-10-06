//  This is TCorrPT library+API (https://bitbucket.org/gems4/tcorrpt)
//
/// \file TCorrPTMenu.cpp
/// TCorrPTWidget - Widget to work with TCorrPT data
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
#include "TCorrPTWidget.h"
#include "ui_TCorrPTWidget.h"
#include "MinMaxDialog.h"
#include "bsonui/SchemaSelectDialog.h"
#include "bsonui/SelectDialog.h"
#include "bsonui/TableEditWindow.h"
#include "bsonui/QueryWidget.h"
#include "bsonio/json2cfg.h"
// TCorrPT include
#include "tcorrpt/TPcalculationsAPI.h"

using namespace bsonio;


//  Connect all actions
void TCorrPTWidget::setActions()
{
    connect( ui->typeBox, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(typeChanged(const QString&)));

    // File
    connect( ui->actionE_xit, SIGNAL( triggered()), this, SLOT(close()));
    connect( ui->actionExport_CfgFile, SIGNAL( triggered()), this, SLOT(CmExportCFG()));
    connect( ui->actionImport_CfgFile, SIGNAL( triggered()), this, SLOT(CmImportCFG()));

    // Edit
    connect(ui->actionCopy_Field_Path, SIGNAL(triggered()), fieldTable, SLOT(CopyFieldPath()));
    connect(ui->actionCopy_Field, SIGNAL(triggered()), fieldTable, SLOT(CopyField()));
    connect(ui->actionRealloc_T, SIGNAL(triggered()), this, SLOT(CmReallocT()));
    connect(ui->actionReset_T, SIGNAL(triggered()), this, SLOT(CmResetT()));
    connect(ui->actionRealloc_P, SIGNAL(triggered()), this, SLOT(CmReallocP()));
    connect(ui->actionReset_P, SIGNAL(triggered()), this, SLOT(CmResetP()));
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
void TCorrPTWidget::CmNew()
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


void TCorrPTWidget::CmDisplaySearchResult()
{
  if( dbgraph.get() == 0 )
       return;
  try
  {
     // define new dialog
     if(!queryResultWindow)
     {
         queryResultWindow = new TableEditWidget("Query Result window",
                         dataTable, tbShow );
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

void TCorrPTWidget::CmSearchQuery()
{
  if( dbgraph.get() == 0 )
       return;

  vector<string> oldkeys = dbgraph->GetQueryFields();
  string oldquery = dbgraph->GetLastQuery();
  try
  {
      // define new dialog
      if(!queryWindow)
      {
          queryWindow = new QueryWidget( "BSONUI Query Widget",
              schema, _shemaNames, oldkeys, oldquery, this, this );
      }
      else
      {
         queryWindow->Update( oldkeys, oldquery);
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

void TCorrPTWidget::setQuery( QueryWidget* queryW  )
{
  if( dbgraph.get() == 0 )
       return;

  vector<string> oldkeys = dbgraph->GetQueryFields();
  string oldquery = dbgraph->GetLastQuery();
  try
  {
      vector<string> lst = queryW->getQueryFields();
      string qrJson = queryW->getQueryJson();

      if( qrJson != oldquery )
      {
        isDefaultQuery = true;
        _data.query = qrJson;

      }
      // reset internal query data
      dbgraph->SetQueryFields(lst);
      dbgraph->SetQueryJson(qrJson);
      dbgraph->runQuery();
      ui->edgeQuery->setText(qrJson.c_str());

      // update search tables
      tableModel->resetMatrixData();
      if( queryResultWindow )
       queryResultWindow->updateTable();
   }
  catch(std::exception& e)
  {
      dbgraph->SetQueryFields( oldkeys );
      dbgraph->SetQueryJson( oldquery );
      QMessageBox::critical( this, "CmSearchQuery", e.what() );
  }
}

// new commands -------------------------------------------------------------------

void TCorrPTWidget::CmReallocT()
{
  try{
        bool ok = 0;
        uint size = QInputDialog::getInt( this, "Please, select new T array size",
                 "Array size ", _data.pointsT.size(), 0, 999, 1, &ok );
        if(!ok) // cancel command
            return;

        _data.pointsT.resize(size);
        _TlistModel->resetMatrixData();
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


void TCorrPTWidget::CmResetT()
{
  try{
        // define new preferences
        MinMaxDialog dlg("Please, insert T interval", this);
         if( !dlg.exec() )
             return;
        int crt = dlg.getMin();
        int Tmax = dlg.getMax();
        int step = dlg.getStep();

        _data.pointsT.clear();
        if(!step)
            _data.pointsT.push_back(crt);
        else
           do{
               _data.pointsT.push_back(crt);
                crt += step;
              }while( (step>0 && crt<=Tmax) || (step<0 && crt>=Tmax));

        _TlistModel->resetMatrixData();
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

void TCorrPTWidget::CmReallocP()
{
  try{
        bool ok = 0;
        uint size = QInputDialog::getInt( this, "Please, select new P array size",
                 "Array size ", _data.pointsP.size(), 0, 999, 1, &ok );
        if(!ok) // cancel command
            return;

        _data.pointsP.resize(size);
        _PlistModel->resetMatrixData();
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


void TCorrPTWidget::CmResetP()
{
  try{
        // define new preferences
        MinMaxDialog dlg("Please, insert P interval", this);
         if( !dlg.exec() )
             return;
        int crt = dlg.getMin();
        int max = dlg.getMax();
        int step = dlg.getStep();

        _data.pointsP.clear();
        if(!step)
            _data.pointsP.push_back(crt);
        else
           do{
               _data.pointsP.push_back(crt);
                crt += step;
              }while( (step>0 && crt<=max) || (step<0 && crt>=max));

        _PlistModel->resetMatrixData();
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

void TCorrPTWidget::CmResetProperty()
{
 try
  {
    SchemaSelectDialog dlg(this, "Please, mark property fields",
        schema, curSchemaName, _data.properties, _data.properties);
    if( !dlg.exec() )
       return;

    _data.properties = dlg.allSelected();
    _data.propertyUnits.resize(_data.properties.size());
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


/// Read bson record from json file fileName
void TCorrPTWidget::CmImportCFG()
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
          resetTCorrPTData();
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
void TCorrPTWidget::CmExportCFG()
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

void TCorrPTWidget::CmCalcMTPARM()
{
    if( dbgraph.get() == 0 )
          return;
   vector<bson> selectedList;

   try {
          // Select keys to send to TCorrPT
          bool isSolvent = false;
          vector<string> aKeyList;
          vector<vector<string>> aValList;
          vector<int> selNdx;
          vector<string> substancesSymbols, substancesClass;
          dbgraph->GetKeyValueList( aKeyList, aValList );

         if( aKeyList.empty() )
             return;

         SelectDialog selDlg( this, "Please, select one or more records", aValList, selNdx );
         if( !selDlg.exec() )
            return;
          selNdx =  selDlg.allSelected();
          TCorrPT::Database tdb;
readData:
          selectedList.resize(selNdx.size());
          substancesSymbols.resize(selNdx.size()); substancesClass.resize(selNdx.size());

         for( uint ii=0; ii<selNdx.size(); ii++ )
         {
           string key = aKeyList[selNdx[ii]];
//           dbgraph->GetRecord( key.c_str() );
//           selectedList.push_back(curRecord);
           dbgraph->GetRecord( key.c_str() );
           string valDB = dbgraph->GetJson();
           jsonToBson( &selectedList[ii], valDB );

           bsonio::bson_to_key( selectedList[ii].data, TCorrPT::substSymbol, substancesSymbols[ii]);
           bsonio::bson_to_key( selectedList[ii].data, TCorrPT::substClass, substancesClass[ii]);
         }

         tdb = TCorrPT::Database (selectedList);

         for (uint ii=0; ii<substancesClass.size(); ii++)
         {
             if (stoi(substancesClass[ii]) == TCorrPT::SubstanceClass::type::AQSOLVENT)
             {
                 tdb.setAllAqSubstanceSolventSymbol(substancesSymbols[ii]);
                 isSolvent = true;
             }
         }

         if (!isSolvent)
         {
 //            selNdx.clear();
             SelectDialog selDlg2( this, "Please, select the solvent (e.g. H2O@) ", aValList, selNdx );
             if( !selDlg2.exec() )
                 return;
             selNdx = selDlg2.allSelected();
             goto readData;
         }

         TCorrPT::TPcalcualationsAPI tpCalc(tdb);
//         tpCalc.calculateThermoProperties(substancesSymbols, {"sm_gibbs_energy"}, 25, 1);
         std::vector<std::vector<double>> TPpairs;
         for (uint jj=0; jj<_data.pointsT.size(); jj++)
         {
             TPpairs.push_back({_data.pointsT[jj], _data.pointsP[jj]});
         }

         // for testing
//         _data.properties.push_back("sm_enthalpy");
         //
         tpCalc.calculateThermoProperties(substancesSymbols, _data.properties, TPpairs);

         cout << "Finished TCorrPT calculation!" << endl;

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

void TCorrPTWidget::CmCalcRTParm()
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
void TCorrPTWidget::CmShowResult()
{
   try {
         cout << "CmShowResult" << endl;
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



// end of TCorrPTMenu.cpp
