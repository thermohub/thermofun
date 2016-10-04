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

    //TCorpt data
    connect( ui->pName, SIGNAL( textEdited(const QString&)), this, SLOT(nameChanged(const QString&)));
    connect( ui->pComment, SIGNAL( textEdited(const QString&)), this, SLOT(commentChanged(const QString&)));
    connect( ui->pTVal, SIGNAL( valueChanged(double)), this, SLOT(TChanged(double)));
    connect( ui->pTunit, SIGNAL( currentIndexChanged(const QString&)), this, SLOT(TUnitsChanged(const QString&)));
    connect( ui->pPVal, SIGNAL( valueChanged(double)), this, SLOT(PChanged(double)));
    connect( ui->pPunit, SIGNAL( currentIndexChanged(const QString&)), this, SLOT(PUnitsChanged(const QString&)));


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

// new commands

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
                     "Please, select Structured Data file", "*.cfg"  ))
        {
            QSettings settings(fileName.c_str(), QSettings::IniFormat);

            TCorrPTData deflt;

            _data.name = settings.value("Name", deflt.name.c_str() ).toString().toUtf8().data();
/*            settings.setValue("Description", _data.comment.c_str() );
            settings.setValue("SchemaName", _data.schemaName.c_str() );
            settings.setValue("Query", _data.query.c_str() );
            settings.setValue("Temperature", _data.T );
            settings.setValue("TemperatureUnits", _data.unitsT.c_str() );
            //QList<double> tplst = QList<double>::fromVector(QVector<double>::fromStdVector(_data.pointsT));
            //settings.setValue("TemperaturePoints", QVariant::fromValue(tplst) );
            settings.setValue("TemperaturePoints", convert2Qt( _data.pointsT).toJson() );
            settings.setValue("Pressure", _data.P );
            settings.setValue("PressureUnits", _data.unitsP.c_str() );
            settings.setValue("PressurePoints", convert2Qt( _data.pointsP).toJson() );

           settings.setValue("PropertiesList", convert2Qt( _data.properties).toJson() );
           settings.setValue("PropertyUnits", convert2Qt( _data.propertyUnits).toJson() );
*/


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

template<>
QJsonDocument convert2Qt( const vector<string> lst)
{
  QJsonArray outlst;
  for(uint ii=0; ii<lst.size(); ii++)
    outlst.append(lst[ii].c_str());
  return QJsonDocument(outlst);
}


/// Write current task to configuration file file
void TCorrPTWidget::CmExportCFG()
{
   try {
         string fileName = _data.name+".cfg";
         if(  ChooseFileSave( this, fileName,
                     "Please, select file to write the data", "*.cfg", fileName  ))
         {
            QSettings settings(fileName.c_str(), QSettings::IniFormat);

            settings.setValue("Name", _data.name.c_str() );
            settings.setValue("Description", _data.comment.c_str() );
            settings.setValue("SchemaName", _data.schemaName.c_str() );
            settings.setValue("Query", _data.query.c_str() );
            settings.setValue("Temperature", _data.T );
            settings.setValue("TemperatureUnits", _data.unitsT.c_str() );
            //QList<double> tplst = QList<double>::fromVector(QVector<double>::fromStdVector(_data.pointsT));
            //settings.setValue("TemperaturePoints", QVariant::fromValue(tplst) );
            settings.setValue("TemperaturePoints", convert2Qt( _data.pointsT).toJson() );
            settings.setValue("Pressure", _data.P );
            settings.setValue("PressureUnits", _data.unitsP.c_str() );
            settings.setValue("PressurePoints", convert2Qt( _data.pointsP).toJson() );
            settings.setValue("PropertiesList", convert2Qt( _data.properties).toJson() );
            settings.setValue("PropertyUnits", convert2Qt( _data.propertyUnits).toJson() );
            settings.sync();
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



// end of EdgesMenu.cpp
