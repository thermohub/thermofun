//  This is TCorrPT library+API (https://bitbucket.org/gems4/tcorrpt)
//
/// \file TCorrPTWidget.cpp
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

#include <boost/regex.hpp>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <iostream>
#include "TCorrPTWidget.h"
#include "ui_TCorrPTWidget.h"
#include "bsonui/TableEditWindow.h"
#include "bsonui/QueryWidget.h"
using namespace bsonio;
using namespace boost;
//----------------------------------------------------------------------

void TCorrPTWidget::closeEvent(QCloseEvent* e)
{
    if( queryResultWindow )
     queryResultWindow->close();

    if( queryWindow )
     queryWindow->close();

    if( !onCloseEvent(this) )
           e->ignore();
       else
           QWidget::closeEvent(e);
}


TCorrPTWidget::TCorrPTWidget(QSettings *amainSettings,ThriftSchema *aschema,
         const string& fileCfgName, QWidget *parent) :
    BSONUIBase(amainSettings, aschema, parent),
    curSchemaName(""),  defaultQuery(""),
    ui(new Ui::TCorrPTWidget),
    dataTable(0), pTable(0), tableModel(0), queryWindow(0), queryResultWindow(0)
{

    _shemaNames.push_back("VertexSubstance");
    _shemaNames.push_back("VertexReaction");
    _shemaNames.push_back("VertexReactionSet");

    // set up default data
    if( curSchemaName.empty())
      curSchemaName = _shemaNames[0];

    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed

    QString title = qApp->applicationName()+" Structured Data Editor and Database Browser";
    setWindowTitle(title);

    //set up main parameters
    bson_init(&curRecord);
    bson_finish(&curRecord);

    //define schema checkbox
    for( uint ii=0; ii<_shemaNames.size(); ii++ )
      ui->typeBox->addItem(_shemaNames[ii].c_str());
    ui->typeBox->setCurrentText(curSchemaName.c_str());

   // define edit tree view
   aHeaderData << "key" << "value"  << "comment" ;
   fieldTable =  new TBsonView(  ui->bsonWidget );
   model_schema = new TSchemaNodeModel(  schema, &curRecord,
        curSchemaName, aHeaderData, this/*ui->centralWidget*/ );
   deleg_schema = new TSchemaNodeDelegate();
   fieldTable->setModel(model_schema);
   fieldTable->setItemDelegate(deleg_schema);
   fieldTable->setColumnWidth( 0, 150 );
   fieldTable->expandToDepth(0);
   ui->gridLayout->addWidget(fieldTable, 1, 0, 1, 2);

   // define all keys tables
   defKeysTables();
   resetDBClient( curSchemaName );

   // define menu
   setActions();
   ui->keySplitter->setStretchFactor(0, 1);
   ui->mainSplitter->setStretchFactor(0, 0);
   ui->mainSplitter->setStretchFactor(1, 0);
   ui->mainSplitter->setStretchFactor(2, 1);

   // reset in/out queries
   if(  pTable->model()->rowCount() > 0 )
   {  // read first line
       openRecordKey( pTable->model()->index(0,0) );
   }else
     CmNew();

}

TCorrPTWidget::~TCorrPTWidget()
{
    // free keys query data
    if(queryResultWindow)
    {
       queryResultWindow->close();
       delete queryResultWindow;
    }
    if(queryWindow)
    {
       queryWindow->close();
       delete queryWindow;
    }
    if( dataTable )
      delete dataTable;
    if( pTable )
      delete pTable;

    delete ui;
    cout << "~TCorrPTWidget" << endl;
}

//-----------------------------------------------------

//
void TCorrPTWidget::resetTypeBox( const QString & text )
{
    disconnect( ui->typeBox, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(typeChanged(const QString&)));
    ui->typeBox->setCurrentText(text);
    connect( ui->typeBox, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(typeChanged(const QString&)));
}

/// Change current Edge
void TCorrPTWidget::typeChanged(const QString & text)
{
  try {
        curSchemaName = text.toUtf8().data();

        bson_destroy( &curRecord );
        bson_init( &curRecord );
        bson_finish( &curRecord );
        model_schema->setupModelData( &curRecord, curSchemaName);
        // fieldTable->expandToDepth(0);

        if(!isDefaultQuery)
        {  resetDBClient(curSchemaName );
           if(  pTable->model()->rowCount() > 0 )
              openRecordKey( pTable->model()->index(0,0), true );
        }
       else
         dbgraph->resetSchema( curSchemaName, false );
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

void TCorrPTWidget::resetDBClient(const string& schemaName_ )
{
  TGraphAbstract* newClient = newDBClient( schemaName_, defaultQuery );
    // no schema
  if( newClient == nullptr )
  {
      dbgraph.reset();
      ui->edgeQuery->setText("");
      return;
  }
  ui->edgeQuery->setText(newClient->GetLastQuery().c_str());
  dbgraph.reset( newClient );
  try{
         dbgraph->Open();
       }
   catch(bsonio_exeption& e)
    {
        cout << "Internal comment " << e.title() << e.what() << endl;
        // QMessageBox::critical( this, e.title(), e.what() );
    }
    catch(std::exception& e)
     {
        QMessageBox::critical( this, "std::exception", e.what() );
     }
    //resetKeysTable();
    resetKeysTable(pTable, dataTable, tableModel , dbgraph.get() );
    dbgraph->setKey( "*" );
}


void TCorrPTWidget::defKeysTables()
{
    // define table of keys
    pTable = new TKeyTable(ui->keySplitter,
                   [=]( const QModelIndex& index ){openRecordKey( index );});
    //TMatrixDelegate* deleg = new TMatrixDelegate();
    //pTable->setItemDelegate(deleg);
    ui->keySplitter->insertWidget(0, pTable);
    ui->keySplitter->setStretchFactor(0, 4);
    //pTable->horizontalHeader()->hide();
    pTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    pTable->setEditTriggers( QAbstractItemView::AnyKeyPressed );
    pTable->setSortingEnabled(true);
    disconnect( pTable, SIGNAL(customContextMenuRequested(QPoint)),
           pTable, SLOT(slotPopupContextMenu(QPoint)));
    connect( pTable, SIGNAL( clicked(const QModelIndex& ) ), this, SLOT(openRecordKey( const QModelIndex& )));
}

void TCorrPTWidget::resetKeysTable(TKeyTable* table_, TKeyListTableNew*& table_data,
     TMatrixModel*& model_ , TGraphAbstract* dbClient )
  {
    if( table_data )
      delete table_data;
    table_data = new TKeyListTableNew("select", dbClient);
    model_ = new TMatrixModel( table_data, this );
    QSortFilterProxyModel *proxyModel;
    proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel( model_ );
    table_->setModel(proxyModel);
    if( table_== pTable )
     table_->hideColumn(0);
}

bool TCorrPTWidget::resetBson(const string& schemaName )
{
   // test legal schema name
   if( !curSchemaName.empty() &&
       !schemaName.empty() && schemaName != curSchemaName )
   return false;
   fieldTable->updateModelData( &curRecord, curSchemaName );
   fieldTable->update();
   return true;
}

void TCorrPTWidget::openRecordKey(  const QModelIndex& rowindex , bool resetInOutQuery  )
{
    // find key
    int row = rowindex.row();
    QModelIndex index = rowindex.sibling( row, 0);
    string key = pTable->model()->data(index).toString().toUtf8().data();

    // Read Record
    openRecordKey(  key, resetInOutQuery  );
}


string TCorrPTWidget::getSchemaFromKey(  bsonio::TGraphAbstract* dbClient, const string& _inV  )
{
    dbClient->GetRecord( (_inV+":").c_str() );
    string label, schemaName = "";
    dbClient->getValue("_label", label);
    if( !( label.empty() || label == S_EMPTY ) )
       schemaName = schema->getVertexName(label);
    return schemaName;
}


void TCorrPTWidget::openRecordKey(  const string& reckey, bool  )
{
   try{
        if( isDefaultQuery )
         dbgraph->resetMode(true);

        // Read Record
        dbgraph->GetRecord( reckey.c_str() );

        if( isDefaultQuery )
        {
           dbgraph->resetMode(false);
           string _schema = dbgraph->GetSchemaName();
           if( _schema != curSchemaName )
           {
               curSchemaName = _schema;
               resetTypeBox( curSchemaName.c_str() );
               //typeChanged(_schema.c_str() );
           }
        }

        //string valDB = dbclient->GetJson();
        bson_destroy( &curRecord );
        //jsonToBson( &curRecord, valDB );
        dbgraph->GetBson(&curRecord);
        resetBson( ""/*dbclient->getKeywd()*/ );
        contentsChanged = false;

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

void TCorrPTWidget::changeKeyList(TKeyTable* table_, TMatrixModel* model_, const string& key)
{
    // reset model data
    model_->resetMatrixData();

    // undefined current record
    if( key.empty() ||  key.find_first_of("*?") != std::string::npos )
      return;

    if( table_ == pTable )
      table_->setColumnHidden(0, false);
    // search `item` text in model
    QModelIndexList Items =  table_->model()->match(  table_->model()->index(0, 0),
       Qt::DisplayRole, QString(key.c_str()));
    if (!Items.isEmpty())
    {
        table_->selectionModel()->select(Items.first(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        table_->scrollTo(Items.first(), QAbstractItemView::EnsureVisible);
    }
    if( table_ == pTable )
      table_->setColumnHidden(0, true);
}

//-------------------------------------------------------------

/// Change current View menu selections
void TCorrPTWidget::updtViewMenu()
{
    ui->action_Show_comments->setChecked( TSchemaNodeModel::showComments );
    ui->action_Display_enums->setChecked(TSchemaNodeModel::useEnumNames);
    ui->action_Edit_id->setChecked(TSchemaNodeModel::editID );
    ui->actionKeep_Data_Fields_Expanded->setChecked(TBsonView::expandedFields );
}

/// Change model ( show/hide comments)
void TCorrPTWidget::updtModel()
{
   model_schema->updateModelData();
   fieldTable->header()->resizeSection(0, 150);
   fieldTable->header()->resizeSection(1, 250);
   fieldTable->expandToDepth(0);
}

/// Change table (Show Enum Names Instead of Values)
void TCorrPTWidget::updtTable()
{
    fieldTable->hide();
    fieldTable->show();
}

/// Update after change DB locations
void TCorrPTWidget::updtDB()
{
    // set up new dbclient
    resetDBClient( curSchemaName );
    // ??? clear current record
    CmNew();
}

/// Change flag "Display Comments to Data Fields"
void TCorrPTWidget::CmShowComments()
{
   showComments( ui->action_Show_comments->isChecked() );
}

/// Change flag "Show Enum Names Instead of Values"
void TCorrPTWidget::CmDisplayEnums()
{
    displayEnums(ui->action_Display_enums->isChecked());
}

/// Change flag "Edit _id of DB Documents (Records) "
void TCorrPTWidget::CmEditID()
{
  editID( ui->action_Edit_id->isChecked());
}

/// Change flag "Keep data fields expanded"
void TCorrPTWidget::CmEditExpanded()
{
  editExpanded( ui->actionKeep_Data_Fields_Expanded->isChecked() );
}

