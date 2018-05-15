#include <iostream>
#include <QMessageBox>
#include "ElementsWidget.h"
#include "ThermoDataSetDialog.h"
#include "ui_ThermoDataSetDialog.h"
#include "jsonui/preferences.h"
#include "thermofun/DBClient/DatabaseClient.h"
#include "thermofun/DBClient/ReactionSetData.h"
#include "thermofun/DBClient/SubstanceData.h"

struct ThermoDataSetDialogPrivate
{
   ThermoDataSetDialog* window;

   /// Selected SourceTDB
   vector<int> sourceTDB;
   /// Define ELEMENTS table data
   vector<ThermoFun::ElementKey> elementsRow;
   /// Define Selected ELEMENTS table data
   vector<ThermoFun::ElementKey> selectedElms;

   /// Connect to Thermo Database Client record
   ThermoFun::DatabaseClient dbclient;

   ///  SubstanceData substData;
   std::shared_ptr<jsonui::StringTable> substData;
   std::shared_ptr<jsonui::TMatrixModel> substModel;
   std::set<string>       allselSubstIds;
   std::set<string>       allselRcsetIds;

   ///  ReactionSetData rcSetData;
   std::shared_ptr<jsonui::StringTable> rcsetData;
   std::shared_ptr<jsonui::TMatrixModel>  rcsetModel;

// ---------------------------------------------

   ThermoDataSetDialogPrivate(ThermoDataSetDialog* awindow):
    window(awindow), dbclient( jsonui::uiSettings().dbclient() )
   {
        auto names = dbclient.substData().getDataNames();
        names.insert( names.begin(), "sourcetdb");
        auto headers = dbclient.substData().getDataHeaders();
        headers.insert( headers.begin(), "sourcetdb");
        auto fieldPaths = dbclient.substData().getDataFieldPaths();
        fieldPaths.insert( fieldPaths.begin(), "properties.sourcetdb");
        dbclient.substData().setDataNamesHeadersFieldpaths( names, headers,fieldPaths );
        substData.reset( new jsonui::StringTable( "records",dbclient.substData().getDataHeaders()) );
        substModel.reset( new jsonui::TMatrixModel( substData.get() ) );

       names = dbclient.reactSetData().getDataNames();
       names.insert( names.begin(), "sourcetdb");
       headers = dbclient.reactSetData().getDataHeaders();
       headers.insert( headers.begin(), "sourcetdb");
       fieldPaths = dbclient.reactSetData().getDataFieldPaths();
       fieldPaths.insert( fieldPaths.begin(), "properties.sourcetdb");
       dbclient.reactSetData().setDataNamesHeadersFieldpaths( names, headers,fieldPaths );
       rcsetData.reset( new jsonui::StringTable( "records", dbclient.reactSetData().getDataHeaders()) );
       rcsetModel.reset( new jsonui::TMatrixModel( rcsetData.get() ) );
   }

   virtual ~ThermoDataSetDialogPrivate()
   { }

   void clearAll()
   {
     elementsRow.clear();
     selectedElms.clear();
     allselSubstIds.clear();
     allselRcsetIds.clear();
   }

   vector<string> getSourcetdbList()
   {
     return dbclient.sourcetdbListAll();
   }

   bool makeAvailableElementsList(const vector<int>& sourcendxs )
   {
      if( sourceTDB == sourcendxs ) // Source did not chaged
          return false;
      clearAll();

      set<ThermoFun::ElementKey> elms;
      for(auto sourcendx: sourcendxs )
      {
        auto elRow = dbclient.availableElementsKey(sourcendx);
        elms.insert(elRow.begin(), elRow.end());
      }
      elementsRow.clear();
      elementsRow.insert(elementsRow.end(), elms.begin(), elms.end());
      sourceTDB = sourcendxs;
      return true;
   }

   const vector<ThermoFun::ElementKey>& allAvailableElementsList() const
   {
     return elementsRow;
   }


   vector<string> makeAvailableElementsIds()
   {
      return dbclient.elementIds(selectedElms);
   }

   void loadSubstanceRecords( const vector<ThermoFun::ElementKey>& elements )
   {
    if( !allselSubstIds.empty() && selectedElms == elements )
        // Source and elements did not chaged
           return;

     selectedElms = elements;
     jsonio::ValuesTable matr;
     for( auto srcTDB: sourceTDB )
     {       auto matrSrc = dbclient.substData().loadRecordsValues(  jsonio::emptyQuery, srcTDB, elements );
       matr.insert(matr.end(), matrSrc.begin(), matrSrc.end());
     }
     // sort data by symbol
     sort(matr.begin(), matr.end(), [](const vector<string>& a, const vector<string>& b) -> bool
     {   if(a[1] == b[1] )
           return a[0] > b[0];
         return a[1] < b[1];
     });

     substData->updateValues( matr );
     substModel->resetMatrixData();
   }

   std::map<string, int> makeAvailableSubstaceList(const set<int>& rows )
   {
       std::map<string, int>  allselSubstSymbols;
       allselSubstIds.clear();
       auto substDataNdx =dbclient.substData().getDataName_DataIndex();

       auto matr = substData->getValues();
       for( int row: rows)
       {
         allselSubstIds.insert( matr[row][substDataNdx["_id"]]);
         allselSubstSymbols[matr[row][substDataNdx["symbol"]]] = 1; // MASTER_PRIMARY
       }
       return allselSubstSymbols;
   }

   void loadReacSetRecords( const vector<ThermoFun::ElementKey>& elements )
   {
       if( !allselRcsetIds.empty() && selectedElms == elements )
           // Source and elements did not chaged
              return;

       selectedElms = elements;
       // read data by elements
       jsonio::ValuesTable matr;
       for( auto srcTDB: sourceTDB )
       {       auto matrSrc =  dbclient.reactSetData().loadRecordsValues(  jsonio::emptyQuery, srcTDB, elements );
         matr.insert(matr.end(), matrSrc.begin(), matrSrc.end());
       }
       /* test substance data selected
       auto rcsetNdx =dbclient.reactSetData().getDataName_DataIndex();;
       jsonio::ValuesTable newmatr;
       for( auto row: matr )
       {
         auto rcsubLst = dbclient.reactSetData().getSubstanceIds( row[rcsetNdx["_id"]]);
         if( jsonio::isSubset(rcsubLst, allselSubstIds) )
             newmatr.push_back(row);
       }*/

       // sort data by symbol
       sort( matr.begin(), matr.end(), [](const vector<string>& a, const vector<string>& b) -> bool
       {   if(a[1] == b[1] )
             return a[0] > b[0];
           return a[1] < b[1];
       });

     rcsetData->updateValues( matr );
     rcsetModel->resetMatrixData();
   }

   std::map<string, int> makeAvailableReactSetIds(const set<int>& rows, std::map<string, int>& rcsetMap )
   {
       std::map<string, int>  allselSubstSymbols;
       std::map<string, int>  idSelSubstSymbols;
       rcsetMap.clear();
       allselRcsetIds.clear();
       auto rcSetDataNdx =dbclient.reactSetData().getDataName_DataIndex();

       auto matr = rcsetData->getValues();
       int type;
       for( int row: rows)
       {
         auto RcSid =  matr[row][rcSetDataNdx["_id"]];
         allselRcsetIds.insert( RcSid);
         jsonio::TArray2Base::string2value( matr[row][rcSetDataNdx["type"]], type );
         rcsetMap[matr[row][rcSetDataNdx["symbol"]]] = type;
         if(  dbclient.reactSetData().getSpeciesMap( RcSid, idSelSubstSymbols ) )
           allselSubstSymbols.insert(idSelSubstSymbols.begin(), idSelSubstSymbols.end() );
       }
       return allselSubstSymbols;
   }


    void saveEdges( const string& idTermoSet, const string& typeTermoSet,
                   const vector<string>& elmIds,  const set<string>& substIds,
                   const set<string>& rcsetIds, bool overwrite );
};


void ThermoDataSetDialogPrivate::saveEdges(  const string& idTermoSet,
    const string& typeTermoSet,
    const vector<string>& elmIds, const set<string>& substIds,
    const set<string>& rcsetIds, bool overwrite )
{

    jsonio::FieldSetMap fillmap;
    unique_ptr<jsonio::TDBEdgeDocument> basisEdge(jsonio::TDBEdgeDocument::newDBEdgeDocument(
                                      jsonui::uiSettings().database(), "EdgeBasis"));
    unique_ptr<jsonio::TDBEdgeDocument> involvesEdge(jsonio::TDBEdgeDocument::newDBEdgeDocument(
                                      jsonui::uiSettings().database(), "EdgeInvolves"));
    unique_ptr<jsonio::TDBEdgeDocument> pullsEdge(jsonio::TDBEdgeDocument::newDBEdgeDocument(
                                      jsonui::uiSettings().database(), "EdgePulls"));


    // EdgeBasis – _outV will be the id of outgoing element vertex
    fillmap.clear();
    fillmap[ "properties.b_type" ] = "element";
    for( auto elmId: elmIds )
       basisEdge->addNewEdge("EdgeBasis", elmId, idTermoSet, fillmap, overwrite  );

    if( typeTermoSet == "LMA_TDS" )
    {
        // EdgeInvolves – _outV will be the id of outgoing reactionset vertex
        fillmap.clear();
        fillmap[ "properties.level_of_data" ] = "coredata";
        for( auto rcsetId: rcsetIds )
           involvesEdge->addNewEdge("EdgeInvolves", rcsetId, idTermoSet, fillmap, overwrite  );
    }
    else
     { // EdgePulls – _outV will be the id of outgoing substance vertex
       fillmap.clear();
       fillmap[ "properties.level_of_data" ] = "coredata";
       for( auto substId: substIds )
          pullsEdge->addNewEdge("EdgePulls", substId, idTermoSet, fillmap, overwrite  );
     }
}


//----------------------------------------------------------------------------

ThermoDataSetDialog::ThermoDataSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ThermoDataSetDialog), data(0), pTable(0),
    pdata(new ThermoDataSetDialogPrivate(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    resetNextButton();
    resetBackButton();

    // init elems
    elmsWidget = new ElementsWidget( ui->page_2 );
    ui->gridLayout_2->addWidget(elmsWidget);

    initThermoDataSetType();
    defineSource();
    defineSubstances();
    defineReactionSets();

    // signals and slots connections
    connect( ui->bBack, SIGNAL(clicked()), this, SLOT(CmBack()));
    connect( ui->bHelp, SIGNAL( clicked() ), this, SLOT( CmHelp() ) );
    connect( ui->bReset, SIGNAL( clicked() ), this, SLOT( CmReset() ) );
    connect( ui->bSelectAll, SIGNAL( clicked() ), this, SLOT( CmSelectAll() ) );
    connect( ui->pCancelButton, SIGNAL( clicked() ),this, SLOT( reject()  ) );
}

ThermoDataSetDialog::~ThermoDataSetDialog()
{
    delete elmsWidget;
    delete pTable;
    delete data;
    delete ui;
}

//------------------- Actions

void ThermoDataSetDialog::CmBack()
{
    ui->stackedWidget->setCurrentIndex( ui->stackedWidget->currentIndex()-1 );
    resetNextButton();
    resetBackButton();
}

void ThermoDataSetDialog::CmNext()
{
  try
  {
    if(ui->stackedWidget->currentIndex()==0)
    {
       defineElements( allSelectedSources() );
    }
    if(ui->stackedWidget->currentIndex()==1)
    {
       if(ui->pType->currentText() == "LMA_TDS")
        ui->stackedWidget->setCurrentIndex ( ui->stackedWidget->currentIndex()+1 );
       else
        updateSubstances();
    }
    if(ui->stackedWidget->currentIndex()==2)
    {
       updateReactionSets();
    }
    ui->stackedWidget->setCurrentIndex ( ui->stackedWidget->currentIndex()+1 );
    resetNextButton();
    resetBackButton();
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

void ThermoDataSetDialog::CmOk()
{
    ThermoDataSetRecordCreate();
    accept();
}


void ThermoDataSetDialog::CmSelectAll()
{
  switch(  ui->stackedWidget->currentIndex() )
  {
    case 0:  pTable->selectAll();
             break;
    case 1:  elmsWidget->CmSelectAll();
             break;
    case 2:  substTable->selectAll();
             break;
    case 3:  rcsetTable->selectAll();
             break;
    default:
             break;
  }
}


void ThermoDataSetDialog::CmReset()
{
  switch(  ui->stackedWidget->currentIndex() )
  {
    case 0:  pTable->clearSelection();
             break;
    case 1:  elmsWidget->CmReset();
             break;
    case 2:  substTable->clearSelection();
             break;
    case 3:  rcsetTable->clearSelection();
             break;
    default:
             break;
  }
}

//------------------------------------------

void  ThermoDataSetDialog::ThermoDataSetRecordCreate()
{
  bool overwrite = true;

  vector<string> elmIds = pdata->makeAvailableElementsIds();
  set<string> substIds;
  set<string> rcSetIds;

  std::map<string, int>  allselSubstSymbols;
  std::map<string, int>  allselRcSetSymbols;
  if( ui->pType->currentText() != "LMA_TDS" )
  {
      allselSubstSymbols = pdata->makeAvailableSubstaceList(substTable->allSelectedRows() );
      substIds = pdata->allselSubstIds;
  }
  else
  {
     allselSubstSymbols = pdata->makeAvailableReactSetIds( rcsetTable->allSelectedRows(), allselRcSetSymbols);
     rcSetIds = pdata->allselRcsetIds;
  }

  string idThermo = SaveThermoDataSet( allselRcSetSymbols, allselSubstSymbols, overwrite );
  string typeTermoSet = ui->pType->currentText().toStdString();

  pdata->saveEdges( idThermo, typeTermoSet, elmIds, substIds, rcSetIds, overwrite );
}


void ThermoDataSetDialog::genSelectedSubstances()
{
  auto selRows = substTable->allSelectedRows();
  pdata->makeAvailableSubstaceList(selRows );
}

string ThermoDataSetDialog::SaveThermoDataSet( const std::map<string, int>&  allselRcSetSymbols,
        const std::map<string, int>&  allselSubstSymbols, bool overwrite )
{
   unique_ptr<jsonio::TDBVertexDocument> thermosetVertex(  jsonio::TDBVertexDocument::newDBVertexDocument(
               jsonui::uiSettings().database(), "VertexThermoDataSet", jsonio::emptyQuery ) );

   jsonio::FieldSetMap fillmap;
   fillmap["properties.symbol"]     = ui->pSymbol->text().toStdString();
   fillmap["properties.stype"]      = to_string(ui->pType->currentData().toInt());
   fillmap["properties.level"]      = ui->pLevel->text().toStdString();
   fillmap["properties.name"]       = ui->pName->text().toStdString();
   fillmap["properties.comment"]    = ui->pComment->text().toStdString();

   fillmap["properties.sourcetdbs"]  = jsonio::vectorToJson(pdata->sourceTDB);
   fillmap["properties.species_map"] = jsonio::mapToJson( allselSubstSymbols );
   fillmap["properties.reactionsets_map"] = jsonio::mapToJson( allselRcSetSymbols );

   string sourceTDBjson = "[ ";
   for( auto elm: pdata->selectedElms )
      sourceTDBjson += " \""+ elm.symbol + "\",";
   sourceTDBjson.pop_back();
   sourceTDBjson += " ]";
   fillmap["properties.element_list"]  = sourceTDBjson;

   set<ThermoFun::ElementKey> elementsset;
   elementsset.insert( pdata->selectedElms.begin(), pdata->selectedElms.end());
   string elementsJsonArray = ElementsToJson( elementsset );
   if( !elementsJsonArray.empty() )
       fillmap[ "properties.elements" ] = elementsJsonArray;

    return thermosetVertex->addNewVertex("VertexThermoDataSet", fillmap, overwrite );
}


void  ThermoDataSetDialog::defineElements( vector<int> sourcetdb )
{
   if( pdata->makeAvailableElementsList(sourcetdb))
   {
       const vector<ThermoFun::ElementKey>& elements = pdata->allAvailableElementsList();
       elmsWidget->setElementList( elements );
   }
}


void 	ThermoDataSetDialog::resetNextButton()
{
    if( ui->stackedWidget->currentIndex() == ui->stackedWidget->count() - 1 ||
        (ui->pType->currentText() != "LMA_TDS" &&
         ui->stackedWidget->currentIndex() >= ui->stackedWidget->count() - 2)  )
    {
        ui->bNext->disconnect();
        connect( ui->bNext, SIGNAL(clicked()), this, SLOT(CmOk()) );
        ui->bNext->setText("&Generate");
    }
    else
    {
        ui->bNext->disconnect();
        connect( ui->bNext, SIGNAL(clicked()), this, SLOT(CmNext()) );
        ui->bNext->setText("&Next>");
    }
}

void 	ThermoDataSetDialog::resetBackButton()
{
    ui->bBack->setEnabled( ui->stackedWidget->currentIndex() > 0 );
}


void ThermoDataSetDialog::defineSource()
{
  vector<string> _list = pdata->getSourcetdbList();

  data = new jsonui::SelectTable("select", _list, '-');
  jsonui::TMatrixModel* model = new jsonui::TMatrixModel( data, this );
  pTable = new jsonui::TMatrixTable(this, jsonui::TMatrixTable::tbNoMenu );
  pTable->setModel(model);
  ui->gridLayout->addWidget(pTable, 6, 0, 1, 6);
  pTable->horizontalHeader()->hide();
  pTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  pTable->setSelectionMode(QAbstractItemView::MultiSelection);
  pTable->setCurrentIndex( pTable->model()->index(0,0) );
  pTable->SelectRow();
}

vector<int> ThermoDataSetDialog::allSelectedSources()
{
    vector<int> arr;
    QModelIndexList selection = pTable->selectionModel()->selectedRows();

    // Multiple rows can be selected
    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        arr.push_back( index.sibling(index.row(),0).data().toInt()  );
    }
    return arr;
}


void  ThermoDataSetDialog::defineReactionSets()
{
    rcsetTable = new jsonui::TMatrixUniqueSelection( 1, ui->page_3 );
    rcsetTable->setModel(pdata->rcsetModel.get());
    rcsetTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    rcsetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    rcsetTable->setSelectionMode(QAbstractItemView::MultiSelection);
    QObject::disconnect( rcsetTable, SIGNAL(customContextMenuRequested(QPoint)),
           rcsetTable, SLOT(slotPopupContextMenu(QPoint)));
    ui->verticalLayout->addWidget(rcsetTable);
}


void  ThermoDataSetDialog::defineSubstances()
{
    substTable = new jsonui::TMatrixUniqueSelection( 1, ui->page_4 );
    substTable->setModel(pdata->substModel.get());
    substTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    substTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    substTable->setSelectionMode(QAbstractItemView::MultiSelection);
    QObject::disconnect( substTable, SIGNAL(customContextMenuRequested(QPoint)),
           substTable, SLOT(slotPopupContextMenu(QPoint)));
    ui->verticalLayout_2->addWidget(substTable);
}

void   ThermoDataSetDialog::updateSubstances()
{
    vector<ThermoFun::ElementKey> elementKeys;
    elmsWidget->allSelected( elementKeys );
    pdata->loadSubstanceRecords( elementKeys );
}


void   ThermoDataSetDialog::updateReactionSets()
{
    vector<ThermoFun::ElementKey> elementKeys;
    elmsWidget->allSelected( elementKeys );
    pdata->loadReacSetRecords( elementKeys );
}

void ThermoDataSetDialog::initThermoDataSetType()
{
    jsonio::ThriftEnumDef* enumdef =  jsonio::ioSettings().Schema()->getEnum("ThermoDataSetType" );
    if(enumdef != nullptr )
    {
        vector<string> lst = enumdef->getNamesList();
        for( uint ii=0; ii<lst.size(); ii++ )
        {
            string itname =lst[ii];
            ui->pType->addItem(tr(itname.c_str()), enumdef->getId(itname) );
            ui->pType->setItemData(ii, enumdef->getDoc(itname).c_str(), Qt::ToolTipRole);
        }
    }
    ui->pType->setCurrentText("GEM_TDS");
}



