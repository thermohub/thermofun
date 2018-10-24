#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include "jsonui/VertexWidget.h"
#include "jsonui/EdgesWidget.h"
#include "jsonui/HelpMainWindow.h"
#include "ThermoFunMainWindow.h"
#include "ui_ThermoFunMainWindow.h"
#include "ThermoFunWidgetNew.h"
#ifdef FROM_SRC
#include "../src/DBClient/formuladata.h"
#else
#include "thermofun/DBClient/formuladata.h"
#endif

TThermoFunMainWin::TThermoFunMainWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TThermoFunMainWindow)
{
   ui->setupUi(this);
   //setWindowIcon(QIcon(":/main/GUI/Icons/thermomatch-logo-icon.png"));

   ui->centralWidget_->setStyleSheet(
    "border-image: url(:/main/GUI/Icons/BackgroundAqueosReactions900.png) stretch;");

   onCloseEvent = [&](QMainWindow* win)
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

   showWidget =  [=]( bool isVertex, const std::string& testschema,
           const std::string& recordkey, const jsonio::DBQueryData& query )
           {
               OpenNewWidget( isVertex, testschema, recordkey, query );
           };


   //set up main parameters
   setActions();

   setAllElements();
   new HelpMainWindow(0);
}

TThermoFunMainWin::~TThermoFunMainWin()
{
    delete ui;
}

//  Connect all actions
void TThermoFunMainWin::setActions()
{
     // File
    connect( ui->actionNew_Element_Window , SIGNAL( triggered()), this, SLOT(CmNewElement()));
    connect( ui->actionNew_Substance_Window , SIGNAL( triggered()), this, SLOT(CmNewSubstance()));
    connect( ui->actionNew_Reaction_Window , SIGNAL( triggered()), this, SLOT(CmNewReaction()));
    connect( ui->actionNew_Reaction_Se_t_Window , SIGNAL( triggered()), this, SLOT(CmNewReactionSet()));
    connect( ui->actionNew_Data_Source_Window , SIGNAL( triggered()), this, SLOT(CmNewDataSource()));
    connect( ui->actionNew_ThermoDataSet_Window , SIGNAL( triggered()), this, SLOT(CmNewThermoDataSet()));
    connect( ui->actionNew_Edge_Editor_Window , SIGNAL( triggered()), this, SLOT(CmNewEdge()));
    connect( ui->actionE_xit, SIGNAL( triggered()), qApp, SLOT(quit()));

     // Help
    connect( ui->action_Help_About, SIGNAL( triggered()), this, SLOT(CmHelpAbout()));
    connect( ui->actionContents, SIGNAL( triggered()), this, SLOT(CmHelpContens()));
    connect( ui->actionAuthors, SIGNAL( triggered()), this, SLOT(CmHelpAuthors()));
    connect( ui->actionLicense, SIGNAL( triggered()), this, SLOT(CmHelpLicense()));
    connect( ui->actionPreferences, SIGNAL( triggered()), &jsonui::uiSettings(), SLOT(CmSettingth()));

    // View
    connect( ui->action_Show_comments, SIGNAL( toggled(bool)), &jsonui::uiSettings(), SLOT(CmShowComments(bool)));
    connect( ui->action_Display_enums, SIGNAL( toggled(bool)), &jsonui::uiSettings(), SLOT(CmDisplayEnums(bool)));
    connect( ui->action_Edit_id, SIGNAL(toggled(bool)), &jsonui::uiSettings(), SLOT(CmEditID(bool)));
    connect( ui->action_Keep_Data_Fields_Expanded, SIGNAL(toggled(bool)), &jsonui::uiSettings(), SLOT(CmEditExpanded(bool)));

    //Data
//    connect( ui->actionGenerate_a_ThermoDataSet_record, SIGNAL( triggered()), this, SLOT(CmGenerateThermoDataSet()));
//    connect( ui->actionExport_ThermoDataSet, SIGNAL( triggered()), this, SLOT(CmExportThermoDataSet()));

    //Tools
    connect( ui->actionThermo_Fun_Properties_at_TP, SIGNAL( triggered()), this, SLOT(CmThermoFun()));
//    connect( ui->actionRecord_Calculator, SIGNAL( triggered()), this, SLOT(CmSelectElementsTest()));

    connect( &jsonui::uiSettings(), SIGNAL(dbChanged()), this, SLOT(setAllElements()));
    connect( &jsonui::uiSettings(), SIGNAL(schemaChanged()), this, SLOT(setAllElements()));

    connect( &jsonui::uiSettings(), SIGNAL(dbChanged()), this, SLOT(updateDB()));
    connect( &jsonui::uiSettings(), SIGNAL(schemaChanged()), this, SLOT(closeAll()));
    connect( &jsonui::uiSettings(), SIGNAL(viewMenuChanged()), this, SLOT(updateViewMenu()));
    connect( &jsonui::uiSettings(), SIGNAL(modelChanged()), this, SLOT(updateModel()));
    connect( &jsonui::uiSettings(), SIGNAL(tableChanged()), this, SLOT(updateTable()));
    connect( &jsonui::uiSettings(), SIGNAL(errorSettings(std::string)), this, SLOT(closeAll()));

}


/// Open new EdgesWidget  or JSONUIWidget  window
void TThermoFunMainWin::OpenNewWidget( bool isVertex, const std::string& testschema,
                        const std::string& recordkey, const jsonio::DBQueryData& query )
{
      jsonui::JSONUIBase* testWidget;
      if( isVertex )
      {
          testWidget = new jsonui::VertexWidget( testschema/*, this*/ );
          connect( (jsonui::VertexWidget*)testWidget, SIGNAL(vertexDeleted()), this, SLOT(onDeleteVertex()));
      }
      else
      {
          testWidget = new jsonui::EdgesWidget( testschema, query/*, this*/ );
          connect( (jsonui::EdgesWidget*)testWidget, SIGNAL(edgeDeleted()), this, SLOT(onDeleteEdge()));
      }

      testWidget->setOnCloseEventFunction(onCloseEvent);
      testWidget->setShowWidgetFunction(showWidget);
      bsonuiWindows.push_back(testWidget);

      if( !recordkey.empty() )
      {    testWidget->openRecordKey(  recordkey  );
           if( isVertex )
              testWidget->changeKeyList();
      }
      testWidget->show();
}


// Open new JSONUIWidget window
void TThermoFunMainWin::newSchemaWin(const char* testschema )
{
  try{
      OpenNewWidget( true, testschema, "" );
    }
   catch(jsonio::jsonio_exception& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

void TThermoFunMainWin::CmHelpContens()
{
  helpWin( "ThermoFun", "" );
}

void TThermoFunMainWin::CmHelpAbout()
{
    helpWin( "AboutThermoFun", "" );
}

void TThermoFunMainWin::CmHelpAuthors()
{
  helpWin( "AuthorsThermoFun", "" );
}

void TThermoFunMainWin::CmHelpLicense()
{
  helpWin( "LicenseThermoFun", "" );
}


/// Open new EdgesWidget window
void TThermoFunMainWin::CmNewEdge()
{
  try{
      OpenNewWidget( false, "", "" );
    }
   catch(jsonio::jsonio_exception& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}


/// Update menu for all windows
void TThermoFunMainWin::updateViewMenu()
{
    ui->action_Show_comments->setChecked( jsonui::TJsonSchemaModel::showComments );
    ui->action_Display_enums->setChecked(jsonui::TJsonSchemaModel::useEnumNames);
    ui->action_Edit_id->setChecked(jsonui::TJsonSchemaModel::editID );

    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateViewMenu(); it++; }
}


/// Update model for all windows
void TThermoFunMainWin::updateModel()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateModel(); it++; }
}


/// Update table for all windows
void TThermoFunMainWin::updateTable()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateTable(); it++; }
}


/// Update DB for all windows
void TThermoFunMainWin::updateDB()
{
     HelpMainWindow::pDia->resetDBClient();
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->updateDB(); it++; }
}

/// close all bson windows
void TThermoFunMainWin::closeAll()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {  (*it)->close(); it = bsonuiWindows.begin(); }

    // bsonuiWindows.clear();
}

void TThermoFunMainWin::onDeleteEdge()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {
       jsonui::EdgesWidget* edgWin = dynamic_cast<jsonui::EdgesWidget*>(*it);
       if( edgWin )
         edgWin->changeKeyList();
       it++;
    }
}

void TThermoFunMainWin::onDeleteVertex()
{
    auto it = bsonuiWindows.begin();
    while( it != bsonuiWindows.end() )
    {
        jsonui::EdgesWidget* edgWin = dynamic_cast<jsonui::EdgesWidget*>(*it);
        if( edgWin )
          edgWin->updateAllKeys();
        else
          {
            jsonui::VertexWidget* verWin = dynamic_cast<jsonui::VertexWidget*>(*it);
            if( verWin )
              verWin->changeKeyList();
          }
        it++;
    }
}

// ----------------------------------------

void TThermoFunMainWin::CmThermoFun()
{
  try{
        ThermoFunWidgetNew* testWidget = new ThermoFunWidgetNew( this );

        testWidget->setOnCloseEventFunction(onCloseEvent);
        //testWidget->setShowWidgetFunction(showWidget);
        bsonuiWindows.push_back(testWidget);
        testWidget->show();
        //testWidget->CmSelectThermoDataSet();

    }
   catch(jsonio::jsonio_exception& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

/// Set up all elements to formula parser
void TThermoFunMainWin::setAllElements()
{
    try{
         if( jsonio::ioSettings().Schema()->getStruct("VertexElement") == nullptr )
            return;

         // Connect to DataBase
         std::unique_ptr<jsonio::TDBVertexDocument> elementDB( jsonio::TDBVertexDocument::newVertexDocumentQuery(
                    jsonui::uiSettings().database(),  "VertexElement" ));

         if(elementDB.get() == nullptr )
           return;

         ThermoFun::ChemicalFormula::setDBElements( elementDB.get(),
                                        ThermoFun::ChemicalFormula::getDefaultQuery() );;
         // test
         const ThermoFun::DBElementsData& elements = ThermoFun::ChemicalFormula::getDBElements();

        // save data to file
        std::fstream outFile("elementsDB_All.csv", std::ios::out );
        jsonio::jsonioErrIf( !outFile.good() , "elementsDB_All.csv", "Fileopen error...");

        outFile << "symbol,class_,isotope,atomic_mass,";
        outFile << "entropy,heat_capacity,volume,valence,number" << std::endl;

        auto itr = elements.begin();
        while( itr != elements.end() )
        {
            outFile << itr->first.symbol << ","<< itr->first.class_ << ",";
            outFile << itr->first.isotope << ","<< itr->second.atomic_mass << ",";
            outFile << itr->second.entropy << ","<< itr->second.heat_capacity << ",";
            outFile << itr->second.volume << ","<< itr->second.valence << ",";
            outFile << itr->second.number;
            outFile << endl;
            itr++;
        }

    }
   catch(jsonio::jsonio_exception& e)
   {
       QMessageBox::critical( this, e.title(), e.what() );
   }
   catch(std::exception& e)
    {
       QMessageBox::critical( this, "std::exception", e.what() );
    }
}

