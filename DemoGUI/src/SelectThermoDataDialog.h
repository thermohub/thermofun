#ifndef SELECTTHERMODIALOG_H
#define SELECTTHERMODIALOG_H

#include <QDialog>
#include <QtWidgets>
#include "jsonui/SelectDialog.h"
// ThermoFun includes
#ifdef FROM_SRC
#include "../src/DBClient/DatabaseClient.h"
#endif
#ifndef FROM_SRC
#include "thermofun/DBClient/DatabaseClient.h"
#endif

namespace Ui {
class SelectThermoData;
}

class SelectThermoDataDialogPrivate;
class ElementsWidget;

class SelectThermoDataDialog : public QDialog
{
    Q_OBJECT

protected slots:

    void objectChanged(){}

    void CmHelp() {}
    void CmFinish();
    void CmNext();
    void CmBack();
    void CmClearSelection();
    void CmSelectAll();

public:

    explicit SelectThermoDataDialog( char acase, ThermoFun::DatabaseClient& dbclient, QWidget *parent = 0);
    ~SelectThermoDataDialog();

    /// Return all changed data
    //const ThermoFun::DatabaseClient& dbclientData() const;
    /// Return all selected elements
    void allSelected( std::vector<ThermoFun::ElementKey>& elementKeys ) const;

private:

    /// There are two cases for selecting data in the widgets that I foresee:
    /// A.    Selection of data starting from ThermoDataSet list – select one ThermoDataSet
    /// B.    Selection of data starting from sourcetdb list – select one or more sourceDB
    char useCase;

    Ui::SelectThermoData *ui;

    // StepA
    /// ThermoDataSet selection table
    jsonui::TMatrixTable *thermoTable = 0;
    // StepB
    /// Selection of data starting from sourcetdb list
    jsonui::SelectTable  *data = 0;
    jsonui::TMatrixTable *sourceDBTable = 0;
    // Step2
    /// Widget to selection of elements
    ElementsWidget* elmsWidget;
    // Step3
    /// SubstanceData::ValuesTable selection table
    jsonui::TMatrixTable* substTable = 0;
    // Step4
    /// ReactionData::ValuesTable selection table
    jsonui::TMatrixTable* reactTable = 0;
    // Step5
    /// ReactionSetData::ValuesTable selection table
    jsonui::TMatrixTable* rcsetTable = 0;

    /// Internal Data
    std::unique_ptr<SelectThermoDataDialogPrivate> pdata;

    void  resetButtons();
    void  setModel( jsonui::TMatrixTable* table, jsonui::TMatrixModel* model );
    std::vector<int> allSelectedRows( jsonui::TMatrixTable *dataTable );

    // define windows
    void  defineTermodata();
    void  defineSourceTDB();
    void  defineSubstance();
    void  defineReaction();
    void  defineReactionSets();

    // updateData
    void  updateElementsThermo();
    void  updateElementsSourceTDBs();
    void  updateSubstance();
    void  updateReaction();
    void  updateReactionSets();

    /// Update all table containers - left only selected values
    void leftOnlySelected();

};

#endif // SELECTTHERMODIALOG_H
