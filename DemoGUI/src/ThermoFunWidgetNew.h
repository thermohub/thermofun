//  This is ThermoFun library+API (https://bitbucket.org/gems4/ThermoFun)
//
/// \file ThermoFunWidget.h
/// ThermoFunWidget - Widget to work with ThermoFun data
//
// JSONUI is a C++ Qt5-based widget library and API aimed at implementing
// the GUI for editing/viewing the structured data kept in a NoSQL database,
// with rendering documents to/from JSON/YAML/XML files, and importing/
// exporting data from/to arbitrary foreign formats (csv etc.). Graphical
// presentation of tabular data (csv format fields) is also possible.
//
// Copyright (c) 2015-2016 Svetlana Dmytriieva (svd@ciklum.com) and
//   Dmitrii Kulik (dmitrii.kulik@psi.ch)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU (Lesser) General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// JSONUI depends on the following open-source software products:
// jsonio (https://bitbucket.org/gems4/jsonio); Qt5 (https://qt.io);
// Qwtplot (http://qwt.sourceforge.net).
//

#ifndef ThermoFunWIDGETNEW_H
#define ThermoFunWIDGETNEW_H

#include "jsonui/JSONUIBase.h"
#ifdef FROM_SRC
#include "../src/DBClient/DatabaseClient.h"
#endif
#ifndef FROM_SRC
#include "thermofun/DBClient/DatabaseClient.h"
#endif

namespace jsonui {
class TableEditWidget;
}

namespace Ui {
class ThermoFunWidget;
}

class ThermoFunWidgetPrivate;
class ThermoFunData;

/// Widget to work with CorrPT data
class ThermoFunWidgetNew : public jsonui::JSONUIBase
{
    Q_OBJECT

    friend class ThermoFunWidgetPrivate;
    std::vector<std::string> _shemaNames;
    std::vector<std::string> _typeNames;

    // Work functions

    /// Set up menu commands
    void setActions();
    void initSourceTDB();
    void resetTypeBox( const QString& text );
    void closeEvent(QCloseEvent* e);

    // update after change preferences
    virtual void updtViewMenu() {}
    virtual void updtModel() {}
    virtual void updtTable() {}
    virtual void updtDB();

    /// Reset new ThermoFun data
    void resetThermoFunData( const ThermoFunData& newdata );
//    void updateElements();

protected slots:

    void typeChanged(const QString & text);

public slots:

    // internal slots
    void openRecordKey(  const std::string& , bool  = false  ) {}
    void openRecordKey(  const QModelIndex&  ) {}
    void changeKeyList() {}
    void objectChanged() {}

    // File
    void CmResetThermoFunData();
    void CmExportCFG();
    void CmImportCFG();

   // Record
    void CmDisplaySearchResult();
    void CmSearchQuery();

    //Edit
    void CmResetTP();
    void CmReallocTP();
    void CmResetProperty();
    void CmSelectElements();
    void CmSetElementsReactions();
    void CmSetElementsReactionSets();

    // Calc
    void CmCalcMTPARM();
    void CmCalcRTParm();

    //Result
    void CmShowResult();

    //Preferences
    void CmCalcSubstFromReact(bool togled);
    void CmCalcReactFromSubst(bool togled);

public:
    explicit ThermoFunWidgetNew( QWidget *parent = 0);
    ~ThermoFunWidgetNew();

    void setQuery( jsonui::QueryWidget* queryW  );

private:

    Ui::ThermoFunWidget *ui;
    std::unique_ptr<ThermoFunWidgetPrivate> pdata;

    // Extern windows
    jsonui::QueryWidget* queryWindow = 0;
    jsonui::TableEditWidget* queryResultWindow = 0;
    jsonui::TableEditWidget* _csvWin = 0;

    bool calcSubstFromReact = false;
    bool calcReactFromSubst = false;

    std::vector<std::string> solventsIds;
};


#endif // ThermoFunWINDOW_H
