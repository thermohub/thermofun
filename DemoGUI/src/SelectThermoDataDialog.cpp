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
   // window data

   /// top level widget
    SelectThermoDataDialog* _window;
   /// Connect to ReactionSet record
   ThermoFun::DatabaseClient& _dbclient;

   std::shared_ptr<ThermoViewModel>  thermoModel;
   std::shared_ptr<ThermoViewModel>  substModel;
   std::shared_ptr<ThermoViewModel>  reactModel;
   std::shared_ptr<ThermoViewModel>  rcsetModel;

   // Selkected data

   /// Selected ThermoDataSet
   string idThermoDataSet;
   /// Selected SourceTDBs
   vector<int> sourceTDBs;

   /// Define ELEMENTS table data
   vector<ThermoFun::ElementKey> elementsRow;

// ---------------------------------------------

   SelectThermoDataDialogPrivate(SelectThermoDataDialog* awindow, ThermoFun::DatabaseClient& adbclient):
    _window(awindow), _dbclient( adbclient )
   {
       thermoModel.reset(new ThermoViewModel( &_dbclient.thermoDataSet() , _window ));
       substModel.reset(new ThermoViewModel( &_dbclient.substData(), _window ));
       reactModel.reset(new ThermoViewModel( &_dbclient.reactData(), _window ));
       rcsetModel.reset(new ThermoViewModel( &_dbclient.reactSetData(), _window ));
   }

   virtual ~SelectThermoDataDialogPrivate()
   { }

   void clearAll()
   {
     elementsRow.clear();
   }

   vector<string> getSourcetdbList()
   {
       return _dbclient.sourcetdbListAll();
   }

   bool makeAvailableElementsListA( int selrow )
   {
       auto matr = thermoModel->getValues();
       string idThermo = matr[selrow][_dbclient.thermoDataSet().getDataName_DataIndex()["_id"]];
       auto elmnts = _dbclient.thermoDataSet().getElementsList(idThermo);
       elementsRow.clear();
       if( !elmnts.empty() )
       {    idThermoDataSet = idThermo;
            elementsRow.insert( elementsRow.begin(), elmnts.begin(), elmnts.end() );
       }
     return true;
   }

   bool makeAvailableElementsListB( const vector<int>& sourcetdbs )
   {
       sourceTDBs  = sourcetdbs;
       auto elmnts = _dbclient.thermoDataSet().selectElementsFromSubstancesGiven( sourceTDBs );
       elementsRow.clear();
       if( !elmnts.empty() )
           elementsRow.insert( elementsRow.begin(), elmnts.begin(), elmnts.end() );
       return true;
   }

   const vector<ThermoFun::ElementKey>& allAvailableElementsList() const
   {
     return elementsRow;
   }

   void loadSubstanceRecords( bool typeA, const vector<ThermoFun::ElementKey>& elements, bool unique )
   {
     vector<string> substanceSymbols;
     if( typeA )
        substanceSymbols = _dbclient.substData().selectGiven( idThermoDataSet, elements, false );
     else
        substanceSymbols = _dbclient.substData().selectGiven( sourceTDBs, elements, unique );

     substModel->loadModeRecords( _dbclient.substData().getValuesTable() );
   }

   void loadReactionRecords( bool typeA, const std::vector<int>& substSelectedRows, bool unique )
   {
     vector<string> reactSymbols;
     auto substanceSymbols = substModel->getColumn( _dbclient.substData().getDataName_DataIndex()["symbol"], substSelectedRows );
     if( typeA )
        reactSymbols = _dbclient.reactData().selectGiven( idThermoDataSet, substanceSymbols );
     else
        reactSymbols = _dbclient.reactData().selectGiven( sourceTDBs, substanceSymbols, unique );

     reactModel->loadModeRecords( _dbclient.reactData().getValuesTable() );
   }

   void loadReacSetRecords( bool typeA, const std::vector<int>& reactSelectedRows, bool unique )
   {
     vector<string> scsetSymbols;
     auto reactSymbols = reactModel->getColumn( _dbclient.reactData().getDataName_DataIndex()["symbol"], reactSelectedRows );
     if( typeA )
        scsetSymbols = _dbclient.reactData().selectGiven( idThermoDataSet, reactSymbols );
     else
        scsetSymbols = _dbclient.reactSetData().selectGiven( sourceTDBs, reactSymbols, unique );

     rcsetModel->loadModeRecords( _dbclient.reactSetData().getValuesTable() );
   }

};


//===========================================================================


SelectThermoDataDialog::SelectThermoDataDialog( char acase, ThermoFun::DatabaseClient& dbclient, QWidget *parent) :
    QDialog(parent), useCase(acase),
    ui(new Ui::SelectThermoData), pdata(new SelectThermoDataDialogPrivate( this, dbclient))
{
    ui->setupUi(this);
    ui->checkUnique->hide();

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
        ui->checkUnique->show();
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
//  The container for substances, reactions, and reactionsSets belonging to the ThermoDataset
//   and containing the selected elements will be field
        case 2: updateSubstance();
// The selected substances will be loaded in the substance container (SubstanceData::ValuesTable),
//  followed by the reactions container (ReactionData::ValuesTable) containing the selected substances
// and reactionSets container (ReactionSetData::ValuesTable) containing the reactions
      case 3: updateReaction();
//  The selected reactions will be loaded in the reactions container (ReactionData::ValuesTable),
// followed by the reactionSets container (ReactionSetData::ValuesTable) containing the selected reactions
      case 4: updateReactionSets();
                break;
     }
     // left only selected in mode
     leftOnlySelected();
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
    ui->bFinish->setEnabled( current > 1 );
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
    table->setSelectionMode(QAbstractItemView::MultiSelection);
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
   if( pdata->makeAvailableElementsListA(thermoTable->currentIndex().row()))
   {
       const vector<ThermoFun::ElementKey>& elements = pdata->allAvailableElementsList();
       //foreach ( ElementKey elm, elements)
       // cout << elm.symbol << endl;
       elmsWidget->setElementList( elements );
   }
}

void  SelectThermoDataDialog::updateElementsSourceTDBs()
{
    std::vector<int> sourcetdbs;
    QModelIndexList indexList = sourceDBTable->selectionModel()->selectedIndexes();
    foreach (QModelIndex index, indexList)
    {
        if(index.column() == 0 )
          sourcetdbs.push_back( index.data().toInt() );
    }
    if( pdata->makeAvailableElementsListB(sourcetdbs) )
    {
       const vector<ThermoFun::ElementKey>& elements = pdata->allAvailableElementsList();
       elmsWidget->setElementList( elements );
    }
}

// -- Returns selection elements array
void SelectThermoDataDialog::allSelected( vector<ThermoFun::ElementKey>& elementKeys ) const
{
    elementKeys.clear();
    elmsWidget->allSelected( elementKeys );
}

void   SelectThermoDataDialog::updateSubstance()
{
    vector<ThermoFun::ElementKey> elementKeys;
    allSelected( elementKeys );
    pdata->loadSubstanceRecords( (useCase=='A'),  elementKeys, ui->checkUnique->isChecked() );
    substTable->selectAll();
}

void   SelectThermoDataDialog::updateReaction()
{
    auto substsel = allSelectedRows( substTable );
    pdata->loadReactionRecords( (useCase=='A'),  substsel, ui->checkUnique->isChecked() );
    reactTable->selectAll();
}


void   SelectThermoDataDialog::updateReactionSets()
{
    auto reactsel = allSelectedRows( reactTable );
    pdata->loadReacSetRecords( (useCase=='A'),  reactsel, ui->checkUnique->isChecked() );
    rcsetTable->selectAll();
}


std::vector<int> SelectThermoDataDialog::allSelectedRows( jsonui::TMatrixTable *dataTable )
{
    std::vector<int> rows;
    QModelIndexList indexList = dataTable->selectionModel()->selectedIndexes();
    foreach (QModelIndex index, indexList)
    {
        if(index.column() == 0 )
          rows.push_back( index.row() );
    }
    return rows;
}

void SelectThermoDataDialog::leftOnlySelected()
{
   auto substsel = allSelectedRows( substTable );
   pdata->substModel->leftOnlySelected(substsel);
   auto reactsel = allSelectedRows( reactTable );
   pdata->reactModel->leftOnlySelected(reactsel);
   auto rcsetsel = allSelectedRows( rcsetTable );
   pdata->rcsetModel->leftOnlySelected(rcsetsel);
}

// ------------------------ OK old




