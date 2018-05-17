#ifndef THERMODATASETDIALOG_H
#define THERMODATASETDIALOG_H

#include <QDialog>
#include "jsonui/SelectDialog.h"
#include "thermofun/DBClient/formuladata.h"

namespace Ui {
class ThermoDataSetDialog;
}

class ThermoDataSetDialogPrivate;
class ElementsWidget;

class ThermoDataSetDialog : public QDialog
{
    Q_OBJECT

    void  resetNextButton();
    void  resetBackButton();

    void initThermoDataSetType();
    void  defineSource();
    void  defineSubstances();
    void  defineReactionSets();

    vector<int> allSelectedSources();
    void  defineElements( vector<int> sourcetdb );
    void  updateSubstances();
    void  genSelectedSubstances();
    void  updateReactionSets();

    string SaveThermoDataSet( const map<string, int>&  allselRcSetSymbols,
                    const map<string, int>&  allselSubstSymbols, bool overwrite );
    void  ThermoDataSetRecordCreate();

protected slots:

    void objectChanged(){}

    void CmHelp() {}
    void CmOk();
    void CmNext();
    void CmBack();
    void CmReset();
    void CmSelectAll();

public:

    explicit ThermoDataSetDialog(QWidget *parent = 0);
    ~ThermoDataSetDialog();

private:

    Ui::ThermoDataSetDialog *ui;

    // Step1
    jsonui::SelectTable  *data;
    jsonui::TMatrixTable *pTable;
    // Step2
    /// Widget to selection of elements
    ElementsWidget* elmsWidget;
    // Step3
    /// Substances selection table
    jsonui::TMatrixUniqueSelection* substTable = 0;
    // Step4
    /// ReactionSet selection table
    jsonui::TMatrixUniqueSelection* rcsetTable = 0;

    /// Internal Data
    std::unique_ptr<ThermoDataSetDialogPrivate> pdata;

};

#endif // THERMODATASETDIALOG_H
