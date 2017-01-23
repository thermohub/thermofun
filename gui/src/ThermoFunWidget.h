//  This is ThermoFun library+API (https://bitbucket.org/gems4/ThermoFun)
//
/// \file ThermoFunWidget.h
/// ThermoFunWidget - Widget to work with ThermoFun data
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

#ifndef ThermoFunWIDGET_H
#define ThermoFunWIDGET_H

#include <QJsonDocument>
#include <QJsonArray>
#include "bsonui/BSONUIBase.h"
#include "bsonui/model_node.h"
#include "bsonio/nservice.h"

// ThermoFun include
//#include "thermofun/TPcalculationsAPI.h"
#include "thermofun/Interfaces/Interface.h"


class TableEditWidget;

namespace Ui {
class ThermoFunWidget;
}

/// ThermoFun data to calculation
struct ThermoFunData
{
    string name;    ///< Task (file) name
    string comment; ///< Task description

    string schemaName; ///< Selected schema
    string query;      ///< Current schema

    double T;               ///< T to MTPARM calculations
    //vector<double> pointsT; ///< T points to RTparam calculation
    string unitsT;          ///< Current units of T

    double P;               ///< P to MTPARM calculations
    //vector<double> pointsP; ///< P points to RTparam calculation
    string unitsP;          ///< Current units of P

    std::vector<std::vector<double>> tppairs;

    vector<string> properties;    ///< Properties names list
    vector<string> propertyUnits; ///< Units of property
    vector<int>    propertyPrecision; ///< Units of property

    map<const string,string> mapUnits = ThermoFun::defaultPropertyUnits;
    map<const string, int>   mapPrecision = ThermoFun::defaultPropertyDigits;

    int pPrecision, tPrecision;

    /// Default values to task
    ThermoFunData();

    /// Write current task to configuration file fileName
    void toBson( bson *obj ) const;
    /// Read current task from configuration file fileName
    void fromBson( const char* bsobj );
};

/// Class for double T P vector container
class TPContainer : public TAbstractDataContainer
{
    vector<string>  _keys;
    vector<vector<double>>& _fields;

 public:

   TPContainer( const char * aname, const vector<string>& akeys, vector<vector<double>>& afields ):
      TAbstractDataContainer(aname),
      _keys(akeys),_fields( afields ) {}

   virtual ~TPContainer() {}

   int rowCount() const
   { return _fields.size();  }

   int columnCount() const
   {
      if(_fields.size()>0 )
       return _fields[0].size();
      else
       return 2;
   }

   QVariant data( int line, int column ) const
   {
      return _fields[line][column];
   }

   bool setData( int line, int column, const QVariant &value )
   {
       _fields[line][column] = value.toDouble();
       return true;
   }

   virtual QString headerData ( int section ) const
   {
       return _keys[section].c_str();
   }

   virtual bool  IsEditable( int /*line*/, int /*column*/ ) const
   { return true; }

   virtual int getType( int /*line*/, int /*column*/ ) const
   { return ftDouble; }

   virtual QString getToolTip( int line, int column ) const
   {
       return (_keys[column]+" "+to_string(line)).c_str();
   }

   void resetData()
   { }
};


/// Class for double T P vector container
class TPropertyContainer : public TAbstractDataContainer
{
    vector<string>& _properties;      ///< Properties names list
    vector<string>& _propertyUnits;   ///< Units of property
    vector<int>& _propertyPrecision; ///< Output formats of property

 public:

   TPropertyContainer( const char * aname, vector<string>& aProperties, vector<string>& aPUnits, vector<int>& aPPrecision ):
      TAbstractDataContainer(aname),
      _properties(aProperties),_propertyUnits( aPUnits ), _propertyPrecision( aPPrecision ) {}

   virtual ~TPropertyContainer() {}

   int rowCount() const
   { return _properties.size();  }

   int columnCount() const
   { return 3; }

   QVariant data( int line, int column ) const
   {
       if(column == 0)
          return _properties[line].c_str();
       else
       if(column == 1)
          return _propertyUnits[line].c_str();
       else
          return _propertyPrecision[line];
   }

   bool setData( int line, int column, const QVariant &value )
   {
       string val = value.toString().toUtf8().data();
       int    vali= value.toInt();
       if(column == 0)
          _properties[line] = val;
       else
       if(column == 1)
          _propertyUnits[line] = val;
       else
          _propertyPrecision[line] = vali;
       return true;
   }

   virtual QString headerData ( int section ) const
   {
       if(section == 0)
           return "Property";
       else
       if(section == 1)
           return "Unit";
       else
           return "Precision";
   }

   virtual bool  IsEditable( int /*line*/, int /*column*/ ) const
   { return true; }

   virtual int getType( int /*line*/, int /*column*/ ) const
   { return ftString; }

   virtual QString getToolTip( int /*line*/, int column ) const
   {
       if(column == 0)
           return "Property";
       else
       if(column == 1)
           return "Unit";
       else
           return "Output precision / digits after decimal point (in Fixed Format)";
   }

   void resetData()
   { }
};


/// Widget to work with CorrPT data
class ThermoFunWidget : public BSONUIBase
{
    Q_OBJECT

    // Internal data
    string curSchemaName = "";
    vector<string> _shemaNames;

    ThermoFunData _data;

    // work params
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
    void resetKeysTable();
    void resetTypeBox( const QString& text );

    void closeEvent(QCloseEvent* e);

    // update after change preferences
    virtual void updtViewMenu();
    virtual void updtModel();
    virtual void updtTable();
    virtual void updtDB();

    /// Reset new ThermoFun data
    void resetThermoFunData();
    void updateQuery( const bsonio::DBQueryDef& query  );

protected slots:

    void typeChanged(const QString & text);
    void TChanged(double val)
    {
      _data.T = val;
    }
    void PChanged(double val)
    {
      _data.P = val;
    }
    void pPChanged(int val)
    {
      _data.pPrecision = val;
    }
    void tPChanged(int val)
    {
      _data.tPrecision = val;
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
    void changeKeyList();
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
    void CmResetTP();
    void CmReallocTP();
    void CmResetProperty();

    // Calc
    void CmCalcMTPARM();
    void CmCalcRTParm();

    //Result
    void CmShowResult();


public:
    explicit ThermoFunWidget( QSettings *amainSettings, ThriftSchema *aschema, QWidget *parent = 0);
    ~ThermoFunWidget();

    void setQuery( QueryWidget* queryW  );

private:

    Ui::ThermoFunWidget *ui;

    // edit record view
    QStringList aHeaderData;
    TSchemaNodeModel* model_schema;
    QItemDelegate *deleg_schema;
    TBsonView* fieldTable;

    // keys list data
    boost::shared_ptr<bsonio::TDBGraph> dbgraph;
    TKeyListTableNew* dataTable;
    TKeyTable* pTable;
    TMatrixModel* tableModel;
    QueryWidget* queryWindow;
    TableEditWidget* queryResultWindow;

    //ThermoFun data to edit;
    TPContainer* _TPContainer;
    TMatrixTable*  _TPlistTable;
    TMatrixModel*  _TPlistModel;

    TPropertyContainer* _PropertyContainer;
    TMatrixTable*  _PropertyTable;
    TMatrixModel*  _PropertyModel;

    TableEditWidget* _csvWin = 0;
};


#endif // ThermoFunWINDOW_H
