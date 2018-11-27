#ifndef SELECTELEMENTSDIALOG_H
#define SELECTELEMENTSDIALOG_H

#include <QDialog>
#include <QtWidgets>
#include "jsonui/SelectDialog.h"

// ThermoFun includes
#ifdef FROM_SRC
#include "../src/DBClient/formuladata.h"
#include "../src/DBClient/DatabaseClient.h"
#include "../src/DBClient/ReactionSetData.h"
#include "../src/DBClient/SubstanceData.h"
#endif
#ifndef FROM_SRC
#include "thermofun/DBClient/formuladata.h"
#include "thermofun/DBClient/DatabaseClient.h"
#include "thermofun/DBClient/ReactionSetData.h"
#include "thermofun/DBClient/SubstanceData.h"
#endif




namespace Ui {
class SelectElements;
}

class SelectElementsDialogPrivate;
class ElementsWidget;

class SelectElementsDialog : public QDialog
{
    Q_OBJECT

    void  resetNextButton();
    void  resetBackButton();

    void  defineSource();
    void  defineElements( int sourcetdb );
    void  defineReactionSets();
    void  updateReactionSets();

protected slots:

    void objectChanged(){}

    void CmHelp() {}
    void CmOk();
    void CmNext();
    void CmBack();
    void CmReset();
    void CmSelectAll();

public:

    explicit SelectElementsDialog(ThermoFun::DatabaseClient dbclient, QWidget *parent = 0);
    ~SelectElementsDialog();

    void setData( int sourcetdb, const vector<ThermoFun::ElementKey>& elementKeys, const string& idRcSet );
    void allSelected( vector<ThermoFun::ElementKey>& elementKeys ) const;
    int sourceTDB() const;
    string idReactionSet() const;

private:

    Ui::SelectElements *ui;

    // Step1
    jsonui::SelectTable  *data;
    jsonui::TMatrixTable *pTable;
    // Step2
    /// Widget to selection of elements
    ElementsWidget* elmsWidget;
    // Step3
    /// ReactionSet selection table
    jsonui::TMatrixTable* rcsetTable = 0;

    /// Internal Data
    std::unique_ptr<SelectElementsDialogPrivate> pdata;

};

#endif // SELECTELEMENTSDIALOG_H
