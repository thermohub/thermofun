#include <iostream>
#include <QMessageBox>
#include <QHeaderView>
#include "SelectThermoDataDialog.h"
#include "ui_SelectThermoDataDialog.h"
#include "ElementsWidget.h"
#include "thermomodel.h"
#include "jsonui/TCSVPage.h"

// ThermoFun includes
#ifdef FROM_SRC
#include "../src/DBClient/ThermoSetData.h"
#include "../src/DBClient/SubstanceData.h"
#include "../src/DBClient/ReactionData.h"
#include "../src/DBClient/ReactionSetData.h"
#else
#include "thermofun/DBClient/ThermoSetData.h"
#include "thermofun/DBClient/SubstanceData.h"
#include "thermofun/DBClient/ReactionData.h"
#include "thermofun/DBClient/ReactionSetData.h"
#endif

using namespace std;
using namespace jsonui;

struct SelectThermoDataDialogPrivate
{
    // Window data ---------------------------

    /// Link to top level widget
    SelectThermoDataDialog* _window;
    /// Connect to DatabaseClient for requests
    ThermoFun::DatabaseClient& _dbclient;

    // Thermodata selection containers
    std::shared_ptr<ThermoViewModel>  thermoModel;
    std::shared_ptr<ThermoViewModel>  substModel;
    std::shared_ptr<ThermoViewModel>  reactModel;
    std::shared_ptr<ThermoViewModel>  rcsetModel;

    // Selected data ---------------------------

    /// Selected ThermoDataSet
    string idThermoDataSet;
    /// Selected SourceTDBs
    vector<int> sourceTDBs;

    /// Define ELEMENTS table data
    vector<ThermoFun::ElementKey> elementsAll;
    /// Selected elements
    vector<ThermoFun::ElementKey> elementsSelected;

    /// Solvent substances Values ( for selected elements )
    jsonio::ValuesTable  solventValues;

    // ---------------------------------------------

    SelectThermoDataDialogPrivate(SelectThermoDataDialog* awindow, ThermoFun::DatabaseClient& adbclient):
        _window(awindow), _dbclient( adbclient ), idThermoDataSet("")
    {
        sourceTDBs.clear();
        thermoModel.reset(new ThermoViewModel( &_dbclient.thermoDataSet() , _window ));
        substModel.reset(new ThermoViewModel( &_dbclient.substData(), _window ));
        reactModel.reset(new ThermoViewModel( &_dbclient.reactData(), _window ));
        rcsetModel.reset(new ThermoViewModel( &_dbclient.reactSetData(), _window ));
    }

    virtual ~SelectThermoDataDialogPrivate()
    { }

    vector<string> getSourcetdbList()
    {
        return _dbclient.sourcetdbListAll();
    }

    bool makeAvailableElementsListA( int selrow )
    {
        auto matr = thermoModel->getValues();
        string idThermo = matr[selrow][_dbclient.thermoDataSet().getDataName_DataIndex()["_id"]];
        auto elmnts = _dbclient.thermoDataSet().getElementsList(idThermo);
        elementsAll.clear();
        if( !elmnts.empty() )
        {    idThermoDataSet = idThermo;
            sourceTDBs  = _dbclient.thermoDataSet().sourceTDBs(idThermoDataSet);
            elementsAll.insert( elementsAll.begin(), elmnts.begin(), elmnts.end() );
        }
        return true;
    }

    bool makeAvailableElementsListB( const vector<int>& sourcetdbs )
    {
        sourceTDBs  = sourcetdbs;
        auto elmnts = _dbclient.thermoDataSet().selectElementsFromSubstancesGiven( sourceTDBs );
        elementsAll.clear();
        if( !elmnts.empty() )
            elementsAll.insert( elementsAll.begin(), elmnts.begin(), elmnts.end() );
        return true;
    }

    const vector<ThermoFun::ElementKey>& allAvailableElementsList() const
    {
        return elementsAll;
    }

    void loadSubstanceRecords( bool typeA, const vector<ThermoFun::ElementKey>& elements, bool unique )
    {
        elementsSelected = elements;
        vector<string> substanceSymbols;
        if( typeA )
            substanceSymbols = _dbclient.substData().selectGiven( idThermoDataSet, elements, false );
        else
            substanceSymbols = _dbclient.substData().selectGiven( sourceTDBs, elements, unique );

        const jsonio::ValuesTable& subData = _dbclient.substData().getValuesTable();

        // build solvents table
        solventValues.clear();
        for( auto subRecord: subData )
        {
            if( subRecord[_dbclient.substData().getDataName_DataIndex()["class_"]]  == "{\"3\":\"SC_AQSOLVENT\"}" )
                solventValues.push_back(subRecord);
        }
        substModel->loadModeRecords( subData );
    }

    void loadReactionRecords( bool typeA, const std::vector<size_t>& substSelectedRows, bool unique )
    {
        vector<string> reactSymbols;
        auto substanceSymbols = substModel->getColumn( _dbclient.substData().getDataName_DataIndex()["symbol"], substSelectedRows );
        if( typeA )
            reactSymbols = _dbclient.reactData().selectGiven( idThermoDataSet, substanceSymbols );
        else
            reactSymbols = _dbclient.reactData().selectGiven( sourceTDBs, substanceSymbols, unique );

        reactModel->loadModeRecords( _dbclient.reactData().getValuesTable() );
    }

    void loadReacSetRecords( bool typeA, const std::vector<size_t>& reactSelectedRows, bool unique )
    {
        vector<string> scsetSymbols;
        auto reactSymbols = reactModel->getColumn( _dbclient.reactData().getDataName_DataIndex()["symbol"], reactSelectedRows );
        if( typeA )
            scsetSymbols = _dbclient.reactSetData().selectGiven( idThermoDataSet, reactSymbols );
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
    updateFrom = 0;
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

SelectThermoDataDialog::SelectThermoDataDialog( const std::string& aThermoDataSet,
                                                const std::vector<ThermoFun::ElementKey>& elementKeys,
                                                ThermoFun::DatabaseClient& dbclient, QWidget *parent ):
    SelectThermoDataDialog('A', dbclient, parent )
{
    // set old selection
    selectA( aThermoDataSet, elementKeys  );
}

SelectThermoDataDialog::SelectThermoDataDialog( const std::vector<int>& sourcetdbs,
                                                const std::vector<ThermoFun::ElementKey>& elementKeys,
                                                ThermoFun::DatabaseClient& dbclient, QWidget *parent ):
    SelectThermoDataDialog('B', dbclient, parent )
{
    // set old selection
    selectB( sourcetdbs, elementKeys  );
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

// -- Returns selection elements array
void SelectThermoDataDialog::allSelected( vector<ThermoFun::ElementKey>& elementKeys ) const
{
    elementKeys.clear();
    elmsWidget->allSelected( elementKeys );
}

//  Selection of data starting from ThermoDataSet
std::string SelectThermoDataDialog::idThermoDataSet() const
{
    return pdata->idThermoDataSet;
}

//  Selection of data starting from sourceTDBs
const std::vector<int>& SelectThermoDataDialog::sourceTDBs() const
{
    return pdata->sourceTDBs;
}

// Get the selected substances container
const jsonio::ValuesTable&  SelectThermoDataDialog::getSubstanceValues() const
{
    return pdata->substModel->getValues();
}

// Get the selected reactions container
const jsonio::ValuesTable&  SelectThermoDataDialog::getReactionValues() const
{
    return pdata->reactModel->getValues();
}

// Get Solvent substances container
const jsonio::ValuesTable&  SelectThermoDataDialog::getSolventValues() const
{
    return pdata->solventValues;
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
    catch(jsonio::jsonio_exception& e)
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
    catch(jsonio::jsonio_exception& e)
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
    connect( sourceDBTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
             this,  SLOT(updateAllFrom1(const QItemSelection &, const QItemSelection &))  );
}


void  SelectThermoDataDialog::setModel( TMatrixTable* table, TMatrixModel* model )
{
    //new
    TSortFilterProxyModel *proxyModel = new TSortFilterProxyModel();
    proxyModel->setSourceModel( model );
    table->setModel(proxyModel);
    //old
    //table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::MultiSelection);
    QObject::disconnect( table, SIGNAL(customContextMenuRequested(QPoint)),
                         table, SLOT(slotPopupContextMenu(QPoint)));
}

void  SelectThermoDataDialog::defineTermodata()
{
    thermoTable = new TMatrixTable( this, TMatrixTable::tbSort );
    setModel( thermoTable, pdata->thermoModel->getModel() );
    thermoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->verticalLayout_3->addWidget(thermoTable);
    pdata->thermoModel->loadModeRecords( jsonio::emptyQuery, -1, {} );
    connect( thermoTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
             this,  SLOT(updateAllFrom0(const QItemSelection &, const QItemSelection &))  );
}

void  SelectThermoDataDialog::defineSubstance()
{
    substTable = new TMatrixTable( this, TMatrixTable::tbSort );
    setModel( substTable, pdata->substModel->getModel() );
    ui->verticalLayout->addWidget(substTable);
    connect( substTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
             this,  SLOT(updateAllFrom3(const QItemSelection &, const QItemSelection &))  );
}

void  SelectThermoDataDialog::defineReaction()
{
    reactTable = new TMatrixTable( this, TMatrixTable::tbSort );
    setModel( reactTable, pdata->reactModel->getModel() );
    ui->verticalLayout_5->addWidget(reactTable);
    connect( reactTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
             this,  SLOT(updateAllFrom4(const QItemSelection &, const QItemSelection &))  );
}

void  SelectThermoDataDialog::defineReactionSets()
{
    rcsetTable = new TMatrixTable( this, TMatrixTable::tbSort );
    setModel( rcsetTable, pdata->rcsetModel->getModel() );
    ui->verticalLayout_6->addWidget(rcsetTable);
}

void  SelectThermoDataDialog::updateElementsThermo()
{
    if( updateFrom > 1) // nothing change on page 0
        return;

    int ndxthermo = 0;
    // new
    TSortFilterProxyModel *proxyModel = dynamic_cast<TSortFilterProxyModel *>(thermoTable->model());
    if( proxyModel )
    {
        ndxthermo = proxyModel->mapToSource(thermoTable->currentIndex()).row();
    }
    else
    {
        // old
        ndxthermo = thermoTable->currentIndex().row();
    }

    if( pdata->makeAvailableElementsListA(ndxthermo))
    {
        const vector<ThermoFun::ElementKey>& elements = pdata->allAvailableElementsList();
        //foreach ( ElementKey elm, elements)
        // cout << elm.symbol << endl;
        elmsWidget->setElementList( elements );
        elmsWidget->selectElementList( pdata->elementsSelected );
        updateFrom = 2;
    }
}

void  SelectThermoDataDialog::updateElementsSourceTDBs()
{
    if( updateFrom > 1) // nothing change on page 0
        return;
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
        elmsWidget->selectElementList( pdata->elementsSelected );
        updateFrom = 2;
    }
}


void   SelectThermoDataDialog::updateSubstance()
{
    vector<ThermoFun::ElementKey> elementKeys;
    allSelected( elementKeys );

    // test change element selected
    if( elementKeys != pdata->elementsSelected )
        updateFrom = min( 2, updateFrom);

    if( updateFrom > 2 ) // nothing change on pages 0-2
        return;

    pdata->loadSubstanceRecords( (useCase=='A'),  elementKeys, ui->checkUnique->isChecked() );
    substTable->selectAll();

    updateFrom = 3;
}

void   SelectThermoDataDialog::updateReaction()
{
    if( updateFrom > 3 ) // nothing change on pages 0-3
        return;

    auto substsel = allSelectedRows( substTable );
    pdata->loadReactionRecords( (useCase=='A'),  substsel, ui->checkUnique->isChecked() );
    reactTable->selectAll();

    updateFrom = 4;
}


void   SelectThermoDataDialog::updateReactionSets()
{
    if( updateFrom > 4 ) // nothing change on pages 0-4
        return;

    auto reactsel = allSelectedRows( reactTable );
    pdata->loadReacSetRecords( (useCase=='A'),  reactsel, ui->checkUnique->isChecked() );
    rcsetTable->selectAll();

    updateFrom = 5;
}

std::vector<size_t> SelectThermoDataDialog::allSelectedRows( jsonui::TMatrixTable *dataTable )
{
    std::vector<size_t> rows;
    QItemSelection selitems = dataTable->selectionModel()->selection();
    // new
    TSortFilterProxyModel *proxyModel = dynamic_cast<TSortFilterProxyModel *>(dataTable->model());
    if( proxyModel )
        selitems =  proxyModel->mapSelectionToSource(selitems);
    QModelIndexList indexList = selitems.indexes();
    // old
    // QModelIndexList indexList = dataTable->selectionModel()->selectedIndexes();
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

void SelectThermoDataDialog::selectRows( jsonui::TMatrixTable *dataTable, const std::vector<int>& rows  )
{
    for(auto row: rows)
    {
        QModelIndex index = dataTable->model()->index(row,0);
        dataTable->selectionModel()->select(index, QItemSelectionModel::Rows|QItemSelectionModel::Select);
    }
}

void SelectThermoDataDialog::selectA( const std::string& aThermoDataSet, const std::vector<ThermoFun::ElementKey>& elementKeys  )
{
    auto row = pdata->thermoModel->findRow( pdata->_dbclient.thermoDataSet().getDataName_DataIndex()["_id"], aThermoDataSet );
    row = max( row, 0 );
    if( thermoTable->model()->rowCount() > 0 )
    {

        TSortFilterProxyModel *proxyModel = dynamic_cast<TSortFilterProxyModel *>(thermoTable->model());
        if( proxyModel )
            thermoTable->setCurrentIndex(  proxyModel->mapFromSource( proxyModel->sourceModel()->index(row,0) ) );
        else // old
            thermoTable->setCurrentIndex( thermoTable->model()->index(row,0) );
    }
    pdata->elementsSelected = elementKeys;
}

void SelectThermoDataDialog::selectB( const std::vector<int>& sourcetdbs, const std::vector<ThermoFun::ElementKey>& elementKeys  )
{
    vector<int> rows;
    for( auto sourcetdb: sourcetdbs )
        for( int ii=0; ii<data->rowCount(); ii++ )
        {
            if( data->data( ii, 0 ).toInt() == sourcetdb )
                rows.push_back(ii);
        }

    if( rows.size() > 0)
        sourceDBTable->clearSelection();
    selectRows( sourceDBTable, rows );
    pdata->elementsSelected = elementKeys;
}

//--------------------------------------------------------------------------------------------------
