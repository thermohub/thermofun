#include <iostream>
#include <QMessageBox>
#include <QHeaderView>
#include "SelectElementsDialog.h"
#include "ElementsWidget.h"
#include "ui_SelectElementsDialog.h"
#include "jsonui/preferences.h"

using namespace jsonui;


struct SelectElementsDialogPrivate
{
   SelectElementsDialog* window;

   /// Selected SourceTDB
   int sourceTDB = -1;
   /// Define ELEMENTS table data
   vector<ThermoFun::ElementKey> elementsRow;
   /// Connect to ReactionSet record
   ThermoFun::DatabaseClient dbclient;
//   ReactionSetData rcSetData;
   std::shared_ptr<jsonui::StringTable> rcsetData;
   std::shared_ptr<jsonui::TMatrixModel>  rcsetModel;

   /// Current id of ReactionSet
   string idReactopnSet = "";

// ---------------------------------------------

   SelectElementsDialogPrivate(SelectElementsDialog* awindow, ThermoFun::DatabaseClient dbclient_):
    window(awindow), dbclient( dbclient_ )
   {
       rcsetData.reset( new jsonui::StringTable( "records", dbclient.reactSetData().getDataHeaders()) );
       rcsetModel.reset( new jsonui::TMatrixModel( rcsetData.get() ) );
   }

   virtual ~SelectElementsDialogPrivate()
   { }

   void clearAll()
   {
     elementsRow.clear();
   }

   vector<string> getSourcetdbList()
   {
       return dbclient.sourcetdbListAll();
   }

   bool makeAvailableElementsList( int sourcendx )
   {
      if( sourceTDB == sourcendx ) // Source did not chaged
          return false;
      elementsRow = dbclient.availableElementsKey(sourcendx);
      sourceTDB = sourcendx;
      return true;
   }

   const vector<ThermoFun::ElementKey>& allAvailableElementsList() const
   {
     return elementsRow;
   }

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
       for( unsigned int ii=1; ii<matr.size(); ii++)
        if( idReactopnSet == matr[ii][4] )
         return ii;
     }
     return 0;
   }

   string getReactionSetId(int ndx ) const
   {
     jsonio::ValuesTable matr = rcsetData->getValues();
     if( ndx< (int)matr.size() && ndx > 0 )
         return matr[ndx][4];
     else
         return "";
   }
};


//===========================================================================


SelectElementsDialog::SelectElementsDialog(ThermoFun::DatabaseClient dbclient, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectElements), data(0), pTable(0),
    pdata(new SelectElementsDialogPrivate(this, dbclient))
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);
    resetNextButton();
    resetBackButton();

    // init elems
    elmsWidget = new ElementsWidget( ui->page_2 );
    ui->gridLayout_2->addWidget(elmsWidget);

    defineSource();
    defineReactionSets();

    // signals and slots connections
    connect( ui->bBack, SIGNAL(clicked()), this, SLOT(CmBack()));
    connect( ui->bHelp, SIGNAL( clicked() ), this, SLOT( CmHelp() ) );
    connect( ui->bReset, SIGNAL( clicked() ), this, SLOT( CmReset() ) );
    connect( ui->bSelectAll, SIGNAL( clicked() ), this, SLOT( CmSelectAll() ) );
    connect( ui->pCancelButton, SIGNAL( clicked() ),this, SLOT( reject()  ) );
}

SelectElementsDialog::~SelectElementsDialog()
{
    delete pTable;
    delete data;
    delete elmsWidget;
    delete ui;
}


//------------------- Actions

void SelectElementsDialog::CmBack()
{
    ui->stackedWidget->setCurrentIndex( ui->stackedWidget->currentIndex()-1 );
    resetNextButton();
    resetBackButton();
}

void SelectElementsDialog::CmNext()
{
  try
  {
    if(ui->stackedWidget->currentIndex()==0)
    {
       int ndx = pTable->currentIndex().sibling(pTable->currentIndex().row(),0).data().toInt();
       defineElements( ndx );
    }
    if(ui->stackedWidget->currentIndex()==1)
    {
      // select
       updateReactionSets();
    }
    ui->stackedWidget->setCurrentIndex ( ui->stackedWidget->currentIndex()+1 );
    resetNextButton();
    resetBackButton();
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

void SelectElementsDialog::CmOk()
{
   accept();
}


void SelectElementsDialog::CmSelectAll()
{
  switch(  ui->stackedWidget->currentIndex() )
  {
    case 1:  elmsWidget->CmSelectAll();
             break;
    default:
             break;
  }
}


void SelectElementsDialog::CmReset()
{
  switch(  ui->stackedWidget->currentIndex() )
  {
    case 1:  elmsWidget->CmReset();
             break;
    default:
             break;
  }
}

//--------------------------------------------------------------------

void SelectElementsDialog::defineSource()
{
  vector<string> _list = pdata->getSourcetdbList();

  data = new SelectTable("select", _list, '-');
  TMatrixModel* model = new TMatrixModel( data, this );
  pTable = new TMatrixTable(this, TMatrixTable::tbNoMenu );
  pTable->setModel(model);
  ui->gridLayout->addWidget(pTable, 2, 0, 1, 1);
  pTable->horizontalHeader()->hide();
  pTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  pTable->setSelectionMode(QAbstractItemView::SingleSelection);
  pTable->setCurrentIndex( pTable->model()->index(0,0) );
  pTable->SelectRow();
}

void  SelectElementsDialog::defineElements( int sourcetdb )
{
   if( pdata->makeAvailableElementsList(sourcetdb))
   {
       const vector<ThermoFun::ElementKey>& elements = pdata->allAvailableElementsList();
       //foreach ( ElementKey elm, elements)
       // cout << elm.symbol << endl;
       elmsWidget->setElementList( elements );
   }
}

void  SelectElementsDialog::defineReactionSets()
{
    rcsetTable = new TMatrixTable( this );
    rcsetTable->setModel(pdata->rcsetModel.get());
    rcsetTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    rcsetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    rcsetTable->setSelectionMode(QAbstractItemView::SingleSelection);
    QObject::disconnect( rcsetTable, SIGNAL(customContextMenuRequested(QPoint)),
           rcsetTable, SLOT(slotPopupContextMenu(QPoint)));
    ui->verticalLayout->addWidget(rcsetTable);
}

void   SelectElementsDialog::updateReactionSets()
{
    vector<ThermoFun::ElementKey> elementKeys;
    allSelected( elementKeys );
    int curndx = pdata->loadReacSetRecords( elementKeys );
    rcsetTable->setCurrentIndex(rcsetTable->model()->index( curndx,0 ));
    rcsetTable->SelectRow();
}

// -- Returns selection array
void SelectElementsDialog::allSelected( vector<ThermoFun::ElementKey>& elementKeys ) const
{
    elementKeys.clear();
    string idrc = idReactionSet();
    if( !idrc.empty() )
    {
       auto ellst = pdata->dbclient.reactSetData().getElementsList(idrc);
       elementKeys.insert( elementKeys.end(), ellst.begin(), ellst.end() );
       return;
    }
    elmsWidget->allSelected( elementKeys );
}

int SelectElementsDialog::sourceTDB() const
{
    return pdata->sourceTDB;
}

string SelectElementsDialog::idReactionSet() const
{
  int ndx = rcsetTable->currentIndex().row();
  return pdata->getReactionSetId( ndx );
}


void SelectElementsDialog::setData( int sourcetdb,
         const vector<ThermoFun::ElementKey>& elementKeys, const string& idRcSet )
{
   if( elementKeys.empty() )
     return;

   pdata->idReactopnSet = idRcSet;
   auto index = pTable->currentIndex();
   for( int ii =0;  ii< pTable->model()->rowCount(); ii++ )
   {
     index = index.sibling( ii,0 );
     if( index.data().toInt() == sourcetdb )
     {
       pTable->setCurrentIndex(index);
       CmNext();
       elmsWidget->selectElementList(elementKeys);
     }
   }
}


// ------------------------ OK old

void 	SelectElementsDialog::resetNextButton()
{

    ui->bReset->setVisible(ui->stackedWidget->currentIndex() == 1 );
    ui->bSelectAll->setVisible(ui->stackedWidget->currentIndex() == 1 );

    if( ui->stackedWidget->currentIndex() == ui->stackedWidget->count() - 1 )
    {
        ui->bNext->disconnect();
        connect( ui->bNext, SIGNAL(clicked()), this, SLOT(CmOk()) );
        ui->bNext->setText("&Finish");
    }
    else
    {
        ui->bNext->disconnect();
        connect( ui->bNext, SIGNAL(clicked()), this, SLOT(CmNext()) );
        ui->bNext->setText("&Next>");
    }
}

void 	SelectElementsDialog::resetBackButton()
{
    ui->bBack->setEnabled( ui->stackedWidget->currentIndex() > 0 );
}


