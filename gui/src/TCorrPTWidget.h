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

#include <QJsonDocument>
#include <QJsonArray>
#include "bsonui/BSONUIBase.h"
#include "bsonui/model_node.h"
#include "bsonio/nservice.h"
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

    /// Default values to task
    TCorrPTData();

    /// Write current task to configuration file fileName
    void savetoCFG( const string& fileName );
    /// Read current task from configuration file fileName
    void readfromCFG( const string& fileName );

    void toBson( bson *obj ) const;
    void fromBson( const char* bsobj );
};

/// Class for double T P vector container
class TPVectorContainer : public TAbstractDataContainer
{
    string  _key;
    vector<double>& _fields;

 public:

   TPVectorContainer( const char * aname, const string& akey, vector<double>& afields ):
      TAbstractDataContainer(aname),
      _key(akey),_fields( afields ) {}

   virtual ~TPVectorContainer() {}

   int rowCount() const
   { return _fields.size();  }

   int columnCount() const
   { return 1; }

   QVariant data( int line, int column ) const
   {
       if(column == 0)
          return _fields[line];
       else
          return "";
   }

   bool setData( int line, int column, const QVariant &value )
   {
       if(column == 0)
           _fields[line] = value.toDouble();
       return true;
   }

   virtual QString headerData ( int /*section*/ ) const
   {
       return _key.c_str();
   }

   virtual bool  IsEditable( int /*line*/, int /*column*/ ) const
   { return true; }

   virtual int getType( int /*line*/, int /*column*/ ) const
   { return ftDouble; }

   virtual QString getToolTip( int /*line*/, int /*column*/ ) const
   {
       return _key.c_str();
   }

   void resetData()
   { }
};


/// Class for double T P vector container
class TPropertyContainer : public TAbstractDataContainer
{
    vector<string>& _properties;      ///< Properties names list
    vector<string>& _propertyUnits;   ///< Units of property
//    vector<string>& _propertyFormats; ///< Output formats of property

 public:

   TPropertyContainer( const char * aname, vector<string>& aProperties, vector<string>& aPUnits ):
      TAbstractDataContainer(aname),
      _properties(aProperties),_propertyUnits( aPUnits ) {}

   virtual ~TPropertyContainer() {}

   int rowCount() const
   { return _properties.size();  }

   int columnCount() const
   { return 2; }

   QVariant data( int line, int column ) const
   {
       if(column == 0)
          return _properties[line].c_str();
       else
          return _propertyUnits[line].c_str();
   }

   bool setData( int line, int column, const QVariant &value )
   {
       string val = value.toString().toUtf8().data();
       if(column == 0)
          _properties[line] = val;
       else
          _propertyUnits[line] = val;
       return true;
   }

   virtual QString headerData ( int section ) const
   {
       if(section == 0)
           return "Property";
       else
           return "Unit";
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
           return "Unit";
   }

   void resetData()
   { }
};


/// Widget to work with CorrPT data
class TCorrPTWidget : public BSONUIBase
{
    Q_OBJECT

    // Internal data
    string curSchemaName = "";
    vector<string> _shemaNames;

    TCorrPTData _data;

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

    /// Reset new TCorrPT data
    void resetTCorrPTData();

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
    void CmResetT();
    void CmReallocT();
    void CmResetP();
    void CmReallocP();
    void CmResetProperty();

    // Calc
    void CmCalcMTPARM();
    void CmCalcRTParm();

    //Result
    void CmShowResult();


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

    //TCorrPT data to edit;
    TPVectorContainer* _TContainer;
    TMatrixTable*  _TlistTable;
    TMatrixModel*  _TlistModel;

    TPVectorContainer* _PContainer;
    TMatrixTable*  _PlistTable;
    TMatrixModel*  _PlistModel;

    TPropertyContainer* _PropertyContainer;
    TMatrixTable*  _PropertyTable;
    TMatrixModel*  _PropertyModel;

    TableEditWidget* _csvWin = 0;
};


template<class T>
QJsonDocument convert2Qt( const vector<T> lst)
{
  QJsonArray outlst;
  for(uint ii=0; ii<lst.size(); ii++)
    outlst.append(lst[ii]);
  return QJsonDocument(outlst);
}

template<>
QJsonDocument convert2Qt( const vector<string> lst);
void convertFromQt( const QJsonArray& inlst, vector<string>& lst);
void convertFromQt( const QJsonArray& inlst, vector<double>& lst);

//template<class T>
//void bson_read_array( const char *obj, const char *name, vector<T>& lst )
//{
//    lst.clear();

//    bson_iterator it;
//    bson_type type;
//    type =  bson_find_from_buffer(&it, obj, name );
//    if( type == BSON_NULL || type ==BSON_EOO )
//      return;
//    bsonio::bsonioErrIf( type != BSON_ARRAY,
//                 "E015BSon: ", string("Must be array ")+name );

//    T value;
//    bson_iterator i;
//    bson_iterator_from_buffer(&i, bson_iterator_value(&it));
//    while (bson_iterator_next(&i))
//    {
//        //bson_type t = bson_iterator_type(&i);
//        const char* key = bson_iterator_key(&i);
//        //const char* data = bson_iterator_value(&i);
//        if( !bsonio::bson_find_value( bson_iterator_value(&it), key, value ) )
//          value = 0;
//        lst.push_back(value);
//    }
// }

#endif // TCORRPTWINDOW_H
