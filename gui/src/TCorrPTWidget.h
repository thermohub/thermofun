//  This is TCorrPT library+API (https://bitbucket.org/gems4/tcorrpt)
//
/// \file TCorrPTWidget.h
/// TCorrPTWidget - Widget to work with TCorrPT data
//
// BSONUI is a C++ Qt5-based widget library and API aimed at implementing
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
// BSONUI depends on the following open-source software products:
// BSONIO (https://bitbucket.org/gems4/bsonio); Qt5 (https://qt.io);
// Qwtplot (http://qwt.sourceforge.net).
//

#ifndef TCORRPTWIDGET_H
#define TCORRPTWIDGET_H

#include "bsonui/BSONUIBase.h"
#include "bsonui/model_node.h"
class TableEditWidget;

namespace Ui {
class TCorrPTWidget;
}

/// TCorrPT data to calculation
struct TCorrPTData
{
    string name;    ///< Task (file) name
    string comment; ///< Task description

    string schemaName; ///< Selected schema
    string query;      ///< Current schema

    double T;               ///< T to MTPARM calculations
    vector<double> pointsT; ///< T points to RTparam calculation
    string unitsT;          ///< Current units of T

    double P;               ///< P to MTPARM calculations
    vector<double> pointsP; ///< P points to RTparam calculation
    string unitsP;          ///< Current units of P

    vector<string> properties;    ///< Properties names list
    vector<string> propertyUnits; ///< Units of property
};


/// Widget to work with bson or internal DOM based on our JSON schemas data
class TCorrPTWidget : public BSONUIBase
{
    Q_OBJECT

    // Internal data
    string curSchemaName = "";
    vector<string> _shemaNames;

    TCorrPTData _data;

    // work params
    string defaultQuery;
    bson curRecord;
    bool contentsChanged = false;
    bool isDefaultQuery = false;

    // Work functions

    /// Set up menu commands
    void setActions();
    /// Set up current bson data to view model
    bool resetBson(const string& schemaName );
    /// Reset current DB client
    void resetDBClient(const string& schemaName );
    /// Set up edges, incoming vertexes and outgoing vertexes keys tables
    void defKeysTables();
    /// Reset keys table model
    void resetKeysTable(TKeyTable* table_, TKeyListTableNew*& table_data,
                        TMatrixModel*& model_ , bsonio::TGraphAbstract* dbClient );
    /// Set query to dbClient
    void setOneQuery( QueryWidget* queryW,
           bsonio::TGraphAbstract* dbClient, TMatrixModel* model_ );
    /// Read record and find Schema name from _label
    string getSchemaFromKey(  bsonio::TGraphAbstract* dbClient, const string& inV  );
    void resetTypeBox( const QString& text );

    void closeEvent(QCloseEvent* e);

    // update after change preferences
    virtual void updtViewMenu();
    virtual void updtModel();
    virtual void updtTable();
    virtual void updtDB();

    void changeKeyList( TKeyTable* table_, TMatrixModel* model_, const string& key );

protected slots:

    void typeChanged(const QString & text);
    void TChanged(const QString & text)
    {
      _data.T = text.toDouble();
    }
    void PChanged(const QString & text)
    {
      _data.P = text.toDouble();
    }
    void TUnitsChanged(const QString & text)
    {
      _data.unitsT = text.toUtf8().data();
    }
    void PUnitsChanged(const QString & text)
    {
      _data.unitsP = text.toUtf8().data();
    }
    void nameChanged(const QString & text)
    {
      _data.name = text.toUtf8().data();
    }
    void commentChanged(const QString & text)
    {
      _data.comment = text.toUtf8().data();
    }

public slots:

    // internal slots
    void openRecordKey(  const string& key, bool resetInOutQuery = false  );
    void openRecordKey(  const QModelIndex& index, bool resetInOutQuery = false );
    void changeKeyList()
    {
      changeKeyList( pTable, tableModel,dbgraph->getKeyFromBson( curRecord.data ) );
    }

    void objectChanged()
       { contentsChanged = true; }

    // View
    void CmShowComments();
    void CmDisplayEnums();
    void CmEditID();
    void CmEditExpanded();

   // Record
    void CmDisplaySearchResult();
    void CmSearchQuery();

    // File
    void CmNew();
    void CmExportCFG();
    void CmImportCFG();

    //Edit
    void CmResetT(){}
    void CmReallocT(){}
    void CmResetP(){}
    void CmReallocP(){}
    void CmResetProperty(){}

    // Calc
    void CmCalcMTPARM(){}
    void CmCalcRTParm(){}

    //Result
    void CmShowResult(){}


public:
    explicit TCorrPTWidget( QSettings *amainSettings, ThriftSchema *aschema,
         const string& fileCfgName="", QWidget *parent = 0);
    ~TCorrPTWidget();

    void setQuery( QueryWidget* queryW  );

private:

    Ui::TCorrPTWidget *ui;

    // edit record view
    QStringList aHeaderData;
    TSchemaNodeModel* model_schema;
    QItemDelegate *deleg_schema;
    TBsonView* fieldTable;

    // keys list data
    boost::shared_ptr<bsonio::TGraphAbstract> dbgraph;
    TKeyListTableNew* dataTable;
    TKeyTable* pTable;
    TMatrixModel* tableModel;
    QueryWidget* queryWindow;
    TableEditWidget* queryResultWindow;

};


#endif // TCORRPTWINDOW_H
