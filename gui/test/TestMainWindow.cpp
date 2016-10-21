#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include "TestMainWindow.h"
#include "ui_TestMainWindow.h"
#include "bsonui/model_node.h"
#include "bsonio/json2cfg.h"
#include "bsonui/SelectDialog.h"
#include "bsonui/PreferencesBSONUI.h"
#include "TCorrPTWidget.h"
#include "bsonio/dbdriverejdb.h"
using namespace bsonio;


void TestMainWindow::setDefValues()
{
   // load main programm settingth
   mainSettings = new QSettings("tcorrpt.ini", QSettings::IniFormat);
   getDataFromPreferences();
}

void TestMainWindow::getDataFromPreferences()
{
  if( !mainSettings)
   return;
  SchemDir =  mainSettings->value("SchemasDirectory", "").toString();
  UserDir = mainSettings->value("UserFolderPath", ".").toString();

  TSchemaNodeModel::showComments = mainSettings->value("ShowComments",
                            TSchemaNodeModel::showComments).toBool();
  ui->action_Show_comments->setChecked(TSchemaNodeModel::showComments);
  TSchemaNodeModel::useEnumNames = mainSettings->value("UseEnumNames",
                            TSchemaNodeModel::useEnumNames).toBool();
  ui->action_Display_enums->setChecked(TSchemaNodeModel::useEnumNames);
  TSchemaNodeModel::editID = mainSettings->value("Edit_id",
                            TSchemaNodeModel::editID).toBool();
  ui->action_Edit_id->setChecked(TSchemaNodeModel::editID);
  TBsonView::expandedFields = mainSettings->value("KeepExpanded",
                            TBsonView::expandedFields).toBool();
  ui->action_Keep_Data_Fields_Expanded->setChecked(TBsonView::expandedFields);

  //LocalDBDir = mainSettings->value("LocalDBPath", "./EJDB/localejdb").toString();
  useLocalDB = mainSettings->value("UseLocalDB", false).toBool();
  LocalDBDir = mainSettings->value("LocalDBDirectory", "./EJDB").toString();
  QString LocalDBName = mainSettings->value("LocalDBName", "localdb").toString();
  QFileInfo file(LocalDBDir, LocalDBName);
  LocalDBDir = file.filePath();
  if( /*useLocalDB &&*/ !LocalDBDir.isEmpty() )
    flEJ.reOpen(file.filePath().toUtf8().data());

}



TestMainWindow::TestMainWindow(QWidget *parent) :
    QMainWindow(parent), useLocalDB(false),
    ui(new Ui::TestMainWindow)
{
   ui->setupUi(this);

   //ui->centralWidget_->setStyleSheet(
   // "border-image: url(:/main/GUI/Icons/BackgroundAqueosReactions900.png) stretch;");

   onCloseEvent = [=](QMainWindow* win)
   {
       auto it = bsonuiWindows.begin();
       while( it != bsonuiWindows.end() )
        if( *it == win ) // pointer
        {  bsonuiWindows.erase(it);
           break;
        } else
            it++;
       return true;
   };

   showWidget =  [=]( bool, const string&,  const string&, const string&  )
           { };

   //set up main parameters
   setDefValues();
   setActions();
   readSchemaDir( SchemDir );

   SchemaNode::_schema = &schema;
}

TestMainWindow::~TestMainWindow()
{
    delete ui;
}

//  Connect all actions
void TestMainWindow::setActions()
{
     // File
    connect( ui->actionNew_TCorrPT_Window , SIGNAL( triggered()), this, SLOT(CmNewTCorPT()));
    //    connect( ui->actionE_xit, SIGNAL( triggered()), this, SLOT(close()));
    connect( ui->actionE_xit, SIGNAL( triggered()), qApp, SLOT(quit()));

     // Help
    connect( ui->action_Help_About, SIGNAL( triggered()), this, SLOT(CmHelpAbout()));
    connect( ui->actionContents, SIGNAL( triggered()), this, SLOT(CmHelpContens()));
    connect( ui->actionAuthors, SIGNAL( triggered()), this, SLOT(CmHelpAuthors()));
    connect( ui->actionLicense, SIGNAL( triggered()), this, SLOT(CmHelpLicense()));
    connect( ui->actionPreferences, SIGNAL( triggered()), this, SLOT(CmSettingth()));

    // View
    connect( ui->action_Show_comments, SIGNAL( changed()), this, SLOT(CmShowComments()));
    connect( ui->action_Display_enums, SIGNAL( triggered()), this, SLOT(CmDisplayEnums()));
    connect( ui->action_Edit_id, SIGNAL( triggered()), this, SLOT(CmEditID()));
    connect( ui->action_Keep_Data_Fields_Expanded, SIGNAL( triggered()), this, SLOT(CmExpanded()));
}

void TestMainWindow::CmSettingth()
{
  try
  {
     QString oldSchemaDir = SchemDir;
     QString oldLocalDBDir = LocalDBDir;
     bool oldUseLoacaDB = useLocalDB;

     // define new preferences
     PreferencesBSONUI dlg(mainSettings);
      if( !dlg.exec() )
          return;
     //get data from settings
      SchemDir =  mainSettings->value("SchemasDirectory", "").toString();
      QString LocalDBDir_ = mainSettings->value("LocalDBDirectory", "./EJDB").toString();
      QString LocalDBName_ = mainSettings->value("LocalDBName", "localdb").toString();
      QFileInfo file(LocalDBDir_, LocalDBName_);
      LocalDBDir = file.filePath();
      useLocalDB = mainSettings->value("UseLocalDB", false).toBool();

    // close all windows and read new schema
    if( oldSchemaDir != SchemDir )
    {
      // close all opened windows
      closeAll();
      readSchemaDir( SchemDir );
      return;
    }

    // connect to new DB, rereaded keys
    if( oldUseLoacaDB != useLocalDB ||
      ( useLocalDB && oldLocalDBDir != LocalDBDir))
    { // for all opened windows
       updateDB();
    }

    // for all opened windows
    updateViewMenu();
    updateModel();

  }
  catch(std::exception& e)
  {
        QMessageBox::critical( this, "CmSettingth", e.what() );
  }
}

void TestMainWindow::CmShowComments()
{
    TSchemaNodeModel::showComments = ui->action_Show_comments->isChecked();
    mainSettings->setValue("ShowComments",  TSchemaNodeModel::showComments );
    mainSettings->sync();

    // for all opened windows
    updateViewMenu();
    updateModel();
}

void TestMainWindow::CmDisplayEnums()
{
    TSchemaNodeModel::useEnumNames = ui->action_Display_enums->isChecked();
    mainSettings->setValue("UseEnumNames",  TSchemaNodeModel::useEnumNames );
    mainSettings->sync();

    // for all opened windows
    updateViewMenu();
    updateTable();
}

void TestMainWindow::CmEditID()
{
    TSchemaNodeModel::editID = ui->action_Edit_id->isChecked();
    mainSettings->setValue("Edit_id",  TSchemaNodeModel::editID );
    mainSettings->sync();

    // for all opened windows
    updateViewMenu();
}

void TestMainWindow::CmExpanded()
{
    TBsonView::expandedFields = ui->action_Keep_Data_Fields_Expanded->isChecked();
    mainSettings->setValue("KeepExpanded",  TBsonView::expandedFields );
    mainSettings->sync();

    // for all opened windows
    updateViewMenu();
}

//------------------------------------------------------------------------

/// Open new EdgesWidget window
void TestMainWindow::CmNewTCorPT()
{
  try{
        BSONUIBase* testWidget;
        testWidget = new TCorrPTWidget( mainSettings, &schema, ""/*cfg file name, this*/ );

        testWidget->setOnCloseEventFunction(onCloseEvent);
        testWidget->setShowWidgetFunction(showWidget);
        connect( testWidget, SIGNAL( viewMenuChanged()), this, SLOT(updateViewMenu()));
        connect( testWidget, SIGNAL( modelChanged()), this, SLOT(updateModel()));
        connect( testWidget, SIGNAL( tableChanged()), this, SLOT(updateTable()));
        bsonuiWindows.push_back(testWidget);
        testWidget->show();

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


//---------------------------------------------------------------------
// Read all thrift schemas from Dir
void TestMainWindow::readSchemaDir( const QString& dirPath )
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


/// Update menu for all windows
void TestMainWindow::updateViewMenu()
{
    ui->action_Show_comments->setChecked( TSchemaNodeModel::showComments );
    ui->action_Display_enums->setChecked(TSchemaNodeModel::useEnumNames);
    ui->action_Edit_id->setChecked(TSchemaNodeModel::editID );

    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateViewMenu(); it++; }
}


/// Update model for all windows
void TestMainWindow::updateModel()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateModel(); it++; }
}


/// Update table for all windows
void TestMainWindow::updateTable()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateTable(); it++; }
}


/// Update DB for all windows
void TestMainWindow::updateDB()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateDB(); it++; }
}

/// close all bson windows
void TestMainWindow::closeAll()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->close(); it = bsonuiWindows.begin(); }

    // bsonuiWindows.clear();
}

