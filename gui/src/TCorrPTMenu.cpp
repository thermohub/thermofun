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
#include "TCorrPTWidget.h"
#include "ui_TCorrPTWidget.h"
#include "bsonui/SelectDialog.h"
#include "bsonui/FormatImportDialog.h"
#include "bsonui/TableEditWindow.h"
#include "bsonui/QueryWidget.h"
#include "bsonui/model_node.h"
#include "bsonio/json2cfg.h"
#include "bsonio/thrift_import.h"
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


/// Read bson record from json file fileName
void TCorrPTWidget::CmImportCFG()
{
  try{
        string fileName;
        if(  ChooseFileOpen( this, fileName,
                     "Please, select Structured Data file", "*.json"  ))
       {
          FJson file( fileName);
          bson_destroy( &curRecord );
          file.LoadBson( &curRecord );
          if( !resetBson(schemafromName( fileName )) )
          {
            model_schema->saveToBson();
            bsonioErr( fileName , "Try to read another schema format file" );
          }
          contentsChanged = true;
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

/// Write bson record to json file fileName
void TCorrPTWidget::CmExportCFG()
{
   try {
         string fileName = fileShemaExt(curSchemaName, "json");
         if(  ChooseFileSave( this, fileName,
                     "Please, select Structured Data file to write the data", "*.json", fileName  ))
         {
            FJson file( fileName);
            file.SaveBson( curRecord.data );
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
          vector<string> schemalst;
          schemalst.push_back(curSchemaName);
          queryWindow = new QueryWidget( "BSONUI Query Widget",
              schema, schemalst, oldkeys, oldquery, this, this );
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

void TCorrPTWidget::setOneQuery( QueryWidget* queryW,
       bsonio::TGraphAbstract* dbClient, TMatrixModel* model_ )
{
    if( dbClient == 0 )
        return;

    vector<string> oldkeys = dbClient->GetQueryFields();
    string oldquery = dbClient->GetLastQuery();
    try
    {
      vector<string> lst = queryW->getQueryFields();
      string qrJson = queryW->getQueryJson();

      // reset internal query data
      dbClient->SetQueryFields(lst);
      dbClient->SetQueryJson(qrJson);
      dbClient->runQuery();
      // update search tables
      model_->resetMatrixData();
    }
    catch(std::exception& e)
    {
      dbClient->SetQueryFields( oldkeys );
      dbClient->SetQueryJson( oldquery );
      QMessageBox::critical( this, "Set Query", e.what() );
   }
}

void TCorrPTWidget::setQuery( QueryWidget* queryW  )
{
  if(  queryW == queryWindow)
  {
    setOneQuery(queryW, dbgraph.get(), tableModel );
    ui->edgeQuery->setText( dbgraph->GetLastQuery().c_str() );
    if( queryResultWindow )
       queryResultWindow->updateTable();
  }
}

// end of EdgesMenu.cpp
