#include <iostream>
#include <QMessageBox>
#include <QHeaderView>
#include "SelectThermoDataDialog.h"
#include "ElementsWidget.h"
#include "ui_SelectThermoDataDialog.h"
#include "jsonui/preferences.h"
#include "thermomodel.h"

// ThermoFun includes
#ifdef FROM_SRC
//#include "../src/DBClient/formuladata.h"
#include "../src/DBClient/ThermoSetData.h"
#include "../src/DBClient/SubstanceData.h"
#include "../src/DBClient/ReactionData.h"
#include "../src/DBClient/ReactionSetData.h"
#endif
#ifndef FROM_SRC
#include "thermofun/DBClient/formuladata.h"
#include "thermofun/DBClient/DatabaseClient.h"
#include "thermofun/DBClient/ReactionSetData.h"
#include "thermofun/DBClient/SubstanceData.h"
#endif


using namespace jsonui;




struct SelectThermoDataDialogPrivate
{
   SelectThermoDataDialog* window;

   /// Selected ThermoDataSet
   int idThermoDataSet = -1;
   /// Selected SourceTDBs
   vector<int> sourceTDBs;

   /// Define ELEMENTS table data
   vector<ThermoFun::ElementKey> elementsRow;

   /// Connect to ReactionSet record
   ThermoFun::DatabaseClient dbclient;

   std::shared_ptr<ThermoViewModel>  thermoModel;
   std::shared_ptr<ThermoViewModel>  substModel;
   std::shared_ptr<ThermoViewModel>  reactModel;
   std::shared_ptr<ThermoViewModel>  rcsetModel;


// ---------------------------------------------

   SelectThermoDataDialogPrivate(SelectThermoDataDialog* awindow, ThermoFun::DatabaseClient dbclient_):
    window(awindow), dbclient( dbclient_ )
   {
       thermoModel.reset(new ThermoViewModel( &dbclient.thermoDataSet() , window ));
       substModel.reset(new ThermoViewModel( &dbclient.substData(), window ));
       reactModel.reset(new ThermoViewModel( &dbclient.reactData(), window ));
       rcsetModel.reset(new ThermoViewModel( &dbclient.reactSetData(), window ));
//       thermoModel.reset(new ThermoViewModel( new ThermoFun::ThermoSetData(dbclient.thermoDataSet() ), window ));
//       substModel.reset(new ThermoViewModel( new ThermoFun::SubstanceData_(dbclient.substData()), window ));
//       reactModel.reset(new ThermoViewModel( new ThermoFun::ReactionData_(dbclient.reactData()), window ));
//       rcsetModel.reset(new ThermoViewModel( new ThermoFun::ReactionSetData_(dbclient.reactSetData()), window ));
   }

   virtual ~SelectThermoDataDialogPrivate()
   { }

   void clearAll()
   {
     elementsRow.clear();
   }

   vector<string> getSourcetdbList()
   {
       return dbclient.sourcetdbListAll();
   }

   bool makeAvailableElementsList( int selrow )
   {
       auto matr = thermoModel->getValues();
       string idThermo = matr[selrow][dbclient.thermoDataSet().getDataName_DataIndex()["_id"]];
       auto elmnts = dbclient.thermoDataSet().getElementsList(idThermo);
       elementsRow.clear();
       elementsRow.insert( elementsRow.begin(), elmnts.begin(), elmnts.end() );
     return true;
   }

   const vector<ThermoFun::ElementKey>& allAvailableElementsList() const
   {
     return elementsRow;
   }

/*
   int loadReacSetRecords( const vector<ThermoFun::ElementKey>& elements )
   {
     jsonio::ValuesTable matr = dbclient.reactSetData().loadRecordsValues(  jsonio::emptyQuery, sourceTDB, elements );

// here
  ///  auto jsonT = dbclient.substData().getJsonBsonRecordVertex("59a7dcedf3830548000000e7:").first;
// here
     matr.insert( matr.begin(), {"All","", "", "",""});
     rcsetData->updateValues( matr );
     rcsetModel->resetMatrixData();
     if( !idReactopnSet.empty() )
     {
       for( uint ii=1; ii<matr.size(); ii++)
        if( idReactopnSet == matr[ii][4] )
         return ii;
     }
     return 0;
   }

   string getReactionSetId(int ndx ) const
   {
     jsonio::ValuesTable matr = rcsetData->getValues();
     if( ndx< matr.size() && ndx > 0 )
         return matr[ndx][4];
     else
         return "";
   }
   */
};


//===========================================================================


SelectThermoDataDialog::SelectThermoDataDialog( char acase, ThermoFun::DatabaseClient dbclient, QWidget *parent) :
    QDialog(parent), useCase(acase),
    ui(new Ui::SelectThermoData), pdata(new SelectThermoDataDialogPrivate( this, dbclient))
{
    ui->setupUi(this);

    if( useCase=='A')
    {
        defineTermodata();
        ui->stackedWidget->setCurrentIndex(0);
    }
    else
    {
        defineSourceTDB();
        ui->stackedWidget->setCurrentIndex(1);
    }
    resetButtons();

    // init elems
    elmsWidget = new ElementsWidget( ui->page_2 );
    ui->gridLayout_2->addWidget(elmsWidget);

    defineSubstance();
    defineReaction();
    defineReactionSets();

    // signals and slots connections
    connect( ui->bBack, SIGNAL(clicked()), this, SLOT(CmBack()));
    connect( ui->bNext, SIGNAL(clicked()), this, SLOT(CmNext()));
    connect( ui->bFinish, SIGNAL(clicked()), this, SLOT(CmFinish()));

    connect( ui->bHelp, SIGNAL( clicked() ), this, SLOT( CmHelp() ) );
    connect( ui->bReset, SIGNAL( clicked() ), this, SLOT( CmClearSelection() ) );
    connect( ui->bSelectAll, SIGNAL( clicked() ), this, SLOT( CmSelectAll() ) );
    connect( ui->pCancelButton, SIGNAL( clicked() ),this, SLOT( reject()  ) );
}

SelectThermoDataDialog::~SelectThermoDataDialog()
{
    if( thermoTable )
       delete thermoTable;
    if( data )
       delete data;
    if( sourceDBTable )
       delete sourceDBTable;
    if( substTable )
       delete substTable;
    if( reactTable )
       delete reactTable;
    if( rcsetTable )
       delete rcsetTable;

    delete elmsWidget;
    delete ui;
}


//------------------- Actions

void SelectThermoDataDialog::CmBack()
{
    ui->stackedWidget->setCurrentIndex( ui->stackedWidget->currentIndex()-1 );
    if( useCase=='A' && ui->stackedWidget->currentIndex()==1 )
       ui->stackedWidget->setCurrentIndex( ui->stackedWidget->currentIndex()-1 );
    resetButtons();
}

void SelectThermoDataDialog::CmNext()
{
  try
  {
    auto current = ui->stackedWidget->currentIndex();
    switch( current )
    {
      case 0:
          updateElementsThermo();
          current++;
        break;
      case 1:
        updateElementsSourceTDBs();
        break;
      case 2:
        updateSubstance();
        break;
      case 3:
        updateReaction();
        break;
      case 4:
        updateReactionSets();
        break;
    }
    ui->stackedWidget->setCurrentIndex ( current+1 );
    resetButtons();
  }
    catch(jsonio::jsonio_exeption& e)
    {
        QMessageBox::critical( this, e.title(), e.what() );
    }
    catch(std::exception& e)
     {
        QMessageBox::critical( this, "std::exception", e.what() );
     }
}

void SelectThermoDataDialog::CmFinish()
{
    try
    {
      auto current = ui->stackedWidget->currentIndex();
      switch( current )
      {
        case 2:
           loadAll();
           break;
        case 3:
           loadFromSubstance();
           break;
        case 4:
           loadFromReaction();
           break;
        case 5:
           loadFromReactionSets();
           break;
     }
     accept();
  }
  catch(jsonio::jsonio_exeption& e)
  {
      QMessageBox::critical( this, e.title(), e.what() );
  }
  catch(std::exception& e)
   {
      QMessageBox::critical( this, "std::exception", e.what() );
   }

}


void SelectThermoDataDialog::CmSelectAll()
{
  switch(  ui->stackedWidget->currentIndex() )
  {
    case 2:  elmsWidget->CmSelectAll();
           break;
    case 1:  sourceDBTable->SelectAll();
           break;
    case 3:  substTable->SelectAll();
           break;
    case 4:  reactTable->SelectAll();
           break;
    case 5:  rcsetTable->SelectAll();
           break;
    default:
             break;
  }
}


void SelectThermoDataDialog::CmClearSelection()
{
  switch(  ui->stackedWidget->currentIndex() )
  {
    case 2:  elmsWidget->CmReset();
          break;
    case 1:  sourceDBTable->clearSelection();
         break;
    case 3:  substTable->clearSelection();
         break;
    case 4:  reactTable->clearSelection();
         break;
    case 5:  rcsetTable->clearSelection();
         break;
    default:
             break;
  }
}

void SelectThermoDataDialog::resetButtons()
{
    auto current = ui->stackedWidget->currentIndex();
    ui->bReset->setVisible( current != 0 );
    ui->bSelectAll->setVisible(current != 0 );
    ui->bBack->setEnabled( current > 1 );
    ui->bNext->setEnabled( current < ui->stackedWidget->count() - 1 );
    ui->bFinish->setEnabled( current > 2 );
}


//--------------------------------------------------------------------

void SelectThermoDataDialog::defineSourceTDB()
{
  vector<string> _list = pdata->getSourcetdbList();

  data = new SelectTable("select", _list, '-');
  TMatrixModel* model = new TMatrixModel( data, this );
  sourceDBTable = new TMatrixTable(this, TMatrixTable::tbNoMenu );
  sourceDBTable->setModel(model);
  ui->verticalLayout_2->addWidget(sourceDBTable);
  sourceDBTable->horizontalHeader()->hide();
  sourceDBTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  sourceDBTable->setSelectionMode(QAbstractItemView::MultiSelection);
  sourceDBTable->setCurrentIndex( sourceDBTable->model()->index(0,0) );
  sourceDBTable->SelectRow();
}


void  SelectThermoDataDialog::setModel( TMatrixTable* table, TMatrixModel* model )
{
    table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    QObject::disconnect( table, SIGNAL(customContextMenuRequested(QPoint)),
           table, SLOT(slotPopupContextMenu(QPoint)));
}

void  SelectThermoDataDialog::defineTermodata()
{
    thermoTable = new TMatrixTable( this );
    setModel( thermoTable, pdata->thermoModel->getModel() );
    thermoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->verticalLayout_3->addWidget(thermoTable);
    pdata->thermoModel->loadModeRecords( jsonio::emptyQuery, -1, {} );
}

void  SelectThermoDataDialog::defineSubstance()
{
    substTable = new TMatrixTable( this );
    //tableModel = new ThermoViewModel( new ThermoFun::SubstanceData_(dbclient.substData()), window );
    //tableModel->updateModel(valuesTable);
    setModel( substTable, pdata->substModel->getModel() );
    ui->verticalLayout->addWidget(substTable);
}

void  SelectThermoDataDialog::defineReaction()
{
    reactTable = new TMatrixTable( this );
    setModel( reactTable, pdata->reactModel->getModel() );
    ui->verticalLayout_5->addWidget(reactTable);
}


void  SelectThermoDataDialog::defineReactionSets()
{
    rcsetTable = new TMatrixTable( this );
    setModel( rcsetTable, pdata->rcsetModel->getModel() );
    ui->verticalLayout_6->addWidget(rcsetTable);
}

void  SelectThermoDataDialog::updateElementsThermo()
{
   if( pdata->makeAvailableElementsList(thermoTable->currentIndex().row()))
   {
       const vector<ThermoFun::ElementKey>& elements = pdata->allAvailableElementsList();
       //foreach ( ElementKey elm, elements)
       // cout << elm.symbol << endl;
       elmsWidget->setElementList( elements );
   }
}

void   SelectThermoDataDialog::updateReactionSets()
{
    vector<ThermoFun::ElementKey> elementKeys;
    allSelected( elementKeys );
    int curndx = 0;//pdata->loadReacSetRecords( elementKeys );
    rcsetTable->setCurrentIndex(rcsetTable->model()->index( curndx,0 ));
    rcsetTable->SelectRow();
}

// -- Returns selection array
void SelectThermoDataDialog::allSelected( vector<ThermoFun::ElementKey>& elementKeys ) const
{
    elementKeys.clear();
    elmsWidget->allSelected( elementKeys );
}


// ------------------------ OK old




