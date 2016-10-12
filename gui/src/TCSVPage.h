#ifndef TCSVPAGE_H
#define TCSVPAGE_H

#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include "bsonui/model_table.h"
#include "bsonui/graph.h"
#include "bsonio/v_json.h"

class GraphDialog;

class TSortFilterProxyModel : public QSortFilterProxyModel
 {
     Q_OBJECT

 public:
     TSortFilterProxyModel(QObject *parent = 0):
         QSortFilterProxyModel(parent)
     { }

 protected:
     bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

 };


/// Class for double T P vector container
class TCSVContainer : public TAbstractDataContainer, public TPlot
{
    QString _csvData;             ///< Current data
    QVector< QString > _colHeads;
    QVector< bool > _colDouble;
    QVector< QVector<QVariant> > _matrix;

    QVector<int> _xcolms;  ///< Abscissa columns list
    QVector<int> _ycolms;  ///< Ordinate columns list

    void matrixFromCsvString( const QString& valueCsv );
    QString matrixToCsvString();

 public:

   TCSVContainer( const char* aname, QString data ):
      TAbstractDataContainer(aname), TPlot()
   {
       matrixFromCsvString( data );
   }

   virtual ~TCSVContainer() {}

   int rowCount() const
   { return _matrix.size();  }

   int columnCount() const
   { return _colHeads.size(); }

   QVariant data( int line, int column ) const
   {
       QVariant val = _matrix.at(line).at(column);
       return val;
   }

   bool setData( int line, int column, const QVariant &value )
   {
       _matrix[line].replace(column, value);
       return true;
   }

   virtual QString headerData ( int section ) const
   {
       QString head = _colHeads[section];
       int ii = _xcolms.indexOf( section );
       if( ii!= -1)
           head+="(x)";
       ii = _ycolms.indexOf( section );
       if( ii!= -1)
           head+="(y)";
       return head;
   }

   virtual bool  IsEditable( int /*line*/, int /*column*/ ) const
   { return true; }

   virtual int getType( int /*line*/, int column ) const
   {
      if(_colDouble[column])
          return ftDouble;
      else
          return ftString;
   }

   virtual QString getToolTip( int /*line*/, int column ) const
   {
       return _colHeads[column];
   }

   void resetData()
   {
       matrixFromCsvString( _csvData );
   }

   void setCsvString( const QString& valueCsv )
   {
      _csvData = valueCsv;
   }

   QString getCsvString()
   {
      return  matrixToCsvString();
   }

// Plot functions

   /// Get number of points for one curve
   int getLineSize() const
   {
     return rowCount();
   }

   /// Get number of Abscissa
   int getAbsNumber() const
   { return _xcolms.size(); }

   /// Get number of curves
   int getLinesNumber() const
   { return _ycolms.size();   }


   /// Test empty value (skip point)
   bool isEmptyValue( const QPointF& ) const;

   /// Get point from one line to paint  (ndxX - column in Abscissa table)
   virtual QPointF getPoint( int line, int ndx, int ndxX );

   /// Get point from one line to paint
   double getPointIso( int, int  ) { return 0; }

   const char* getName( int line ) const
   {
      if(line<_ycolms.size())
       return  _colHeads[_ycolms[line]].toUtf8().data();
      else
        return 0;
   }

   void toggleX( int ncolmn );
   void toggleY( int ncolmn );

};

class TCSVPage : public QWidget
{
    Q_OBJECT

    QString _path;
    QString _fname;

    // widget
    QVBoxLayout*   _vLayout;
    TCSVContainer* _data;
    TMatrixModel*  _modelCSV;   ///< Model reading csv data
    TMatrixTable*  _tableCSV;   ///< Table for edit csv data

    //Graphic
    struct GraphData* _grdata;    /// last definition of graphic
    GraphDialog *_graph_dlg;

public slots:

  void CmPlotTable(){}
  void CmPrintTable(){}
  void CmFindFromPlot(){}
  void ToggleX(){}
  void ToggleY(){}

  void CmCalc()
    { _tableCSV->CmCalc();  }
  void SelectRow()
    { _tableCSV->SelectRow();  }
  void SelectColumn()
    { _tableCSV->SelectColumn();  }
  void SelectAll()
    { _tableCSV->selectAll();  }
  void CutData()
    { _tableCSV->CutData();  }
  void ClearData()
    { _tableCSV->ClearData();  }
  void CopyData()
    { _tableCSV->CopyData();  }
  void CopyDataHeader()
    { _tableCSV->CopyDataHeader();  }
  void PasteData()
    { _tableCSV->PasteData();  }
  void PasteTransposedData()
    { _tableCSV->PasteTransposedData();  }

public:

    explicit TCSVPage(const char* apath, QWidget *parent = 0);
    ~TCSVPage();

    void readCSVFile( const QString& dir );
    void saveCSVFile( const QString& dir );

    const char *getName() const
    {
      return _fname.toUtf8().data();
    }

    // get graph info
    void showGraphData( QSortFilterProxyModel *pmodel, const string& title );
    void closeGraph();

};

#endif // TCSVPAGE_H
