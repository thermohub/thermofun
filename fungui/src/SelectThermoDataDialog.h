#ifndef SELECTTHERMODIALOG_H
#define SELECTTHERMODIALOG_H

#include <QDialog>
#include <QtWidgets>
#include "jsonui/SelectDialog.h"
// ThermoFun includes
#ifdef FROM_SRC
#include "../src/DBClient/DatabaseClient.h"
#else
#include "thermofun/DBClient/DatabaseClient.h"
#endif

namespace Ui {
class SelectThermoData;
}

struct SelectThermoDataDialogPrivate;
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

   void updateAllFrom0(const QItemSelection &, const QItemSelection &)
   {
     updateFrom = 0;
   }
   void updateAllFrom1(const QItemSelection &, const QItemSelection &)
   {
     updateFrom = 1;
   }
   void updateAllFrom3(const QItemSelection &, const QItemSelection &)
   {
     updateFrom = std::min(3,updateFrom );
   }
   void updateAllFrom4(const QItemSelection &, const QItemSelection &)
   {
     updateFrom = std::min(4,updateFrom );
   }

public:

    explicit SelectThermoDataDialog( char acase, ThermoFun::DatabaseClient& dbclient, QWidget *parent = nullptr);

    SelectThermoDataDialog( const std::string& aThermoDataSet, const std::vector<ThermoFun::ElementKey>& elementKeys,
                            ThermoFun::DatabaseClient& dbclient, QWidget *parent = nullptr);
    SelectThermoDataDialog( const std::vector<int>& sourcetdbs, const std::vector<ThermoFun::ElementKey>& elementKeys,
                            ThermoFun::DatabaseClient& dbclient, QWidget *parent = nullptr);

    ~SelectThermoDataDialog();

    /// Return all selected elements
    void allSelected( std::vector<ThermoFun::ElementKey>& elementKeys ) const;

    ///  Selection of data starting from ThermoDataSet
    std::string idThermoDataSet() const;

    ///  Selection of data starting from sourceTDBs
    const std::vector<int>& sourceTDBs() const;

    /// Get the selected substances container
    const jsonio::ValuesTable&  getSubstanceValues() const;

    /// Get the selected reactions container
    const jsonio::ValuesTable&  getReactionValues() const;

    /// Get Solvent substances container
    const jsonio::ValuesTable&  getSolventValues() const;

    /// Get the selected reactions container
    const jsonio::ValuesTable&  getReactionSetValues() const;


private:

    /// There are two cases for selecting data in the widgets that I foresee:
    /// A.    Selection of data starting from ThermoDataSet list – select one ThermoDataSet
    /// B.    Selection of data starting from sourcetdb list – select one or more sourceDB
    char useCase;
    /// Flag for reset old selection
    int updateFrom = 0;

    Ui::SelectThermoData *ui;

    // StepA
    /// ThermoDataSet selection table
    jsonui::TMatrixTableProxy *thermoTable = nullptr;
    // StepB
    /// Selection of data starting from sourcetdb list
    jsonui::SelectTable  *data = nullptr;
    jsonui::TMatrixTable *sourceDBTable = nullptr;
    // Step2
    /// Widget to selection of elements
    ElementsWidget* elmsWidget;
    // Step3
    /// SubstanceData::ValuesTable selection table
    jsonui::TMatrixTableProxy* substTable = nullptr;
    // Step4
    /// ReactionData::ValuesTable selection table
    jsonui::TMatrixTableProxy* reactTable = nullptr;
    // Step5
    /// ReactionSetData::ValuesTable selection table
    jsonui::TMatrixTableProxy* rcsetTable = nullptr;

    /// Internal Data
    std::unique_ptr<SelectThermoDataDialogPrivate> pdata;

    void  resetButtons();
    void  setModel( jsonui::TMatrixTable* table, jsonui::TMatrixModel* model );
    void selectRows( jsonui::TMatrixTable *dataTable, const std::vector<int>& rows  );

    // define windows
    void  defineTermodata();
    void  defineSourceTDB();
    void  defineSubstance();
    void  defineReaction();
    void  defineReactionSets();
    void selectA( const std::string& aThermoDataSet, const std::vector<ThermoFun::ElementKey>& elementKeys  );
    void selectB( const std::vector<int>& sourcetdbs, const std::vector<ThermoFun::ElementKey>& elementKeys  );

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
