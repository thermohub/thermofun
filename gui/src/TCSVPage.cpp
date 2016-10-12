#include <iostream>
#include <QFile>
#include <QFileInfo>
#include <QPrintDialog>
#include "TCSVPage.h"
//#include "bsonui/GraphDialog.h"

void removeComments( QString& valCsv )
{
    int foundStart = valCsv.indexOf('#');
    int foundEnd;
    while( foundStart >= 0 )
    {
      foundEnd = valCsv.indexOf('\n');
      if( foundEnd > 0 )
        valCsv.remove( foundStart, foundEnd-foundStart+1 );
      else
        {
           valCsv.remove( foundStart);
           break;
        }
      foundStart = valCsv.indexOf('#', foundStart );
    }

}

bool TSortFilterProxyModel::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    TMatrixModel* model =(TMatrixModel *)sourceModel();
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    ///if (left.column() >= model->getNumberStringColumns() )
    ///   return leftData.toDouble() < rightData.toDouble();
    /// else
        return QSortFilterProxyModel::lessThan( left, right);
}

// TCSVContainer--------------------------------------------------

// Set string to internal data
void TCSVContainer::matrixFromCsvString( const QString& valueCsv )
{
   int ii, jj, nlines;

   // clear old data
  _colHeads.clear();
  _matrix.clear();
  _colDouble.clear();
  _xcolms.clear();
  _ycolms.clear();

  if(valueCsv.isEmpty())
      return;

  // split rows
  const QStringList rows = valueCsv.split('\n', QString::KeepEmptyParts);

  // get header
  QStringList cells = rows[0].split(',', QString::KeepEmptyParts);
  for( ii=0; ii< cells.count(); ii++ )
    {   _colHeads.push_back( cells[ii] );
        _colDouble.push_back(true);  //number
    }

  // get values
  for( jj=1, nlines=0;  jj<rows.count(); jj++ )
  {
      cells = rows[jj].split(',', QString::KeepEmptyParts);
      if(cells.count() < _colHeads.size() )
        continue;
      QVector<QVariant> vec;
      bool ok;
      _matrix.push_back( vec );
      for( ii=0; ii<cells.count(); ii++ )
      {
        if( !cells.empty() ||  _colDouble[ii] )
        { cells[ii].toDouble(&ok);
          if( !ok )
          _colDouble[ii] = false;
        }
        _matrix[nlines].push_back( cells[ii] );
      }
      nlines++;
  }

}

QString TCSVContainer::matrixToCsvString( )
{
    QString valCsv;

    if(_colHeads.size() == 0 )
      return "";

    QVectorIterator<QString> head(_colHeads);
    while (head.hasNext())
    {
        valCsv += head.next();
        if(head.hasNext() )
           valCsv += ",";
    }
    valCsv += "\n";

    QVectorIterator<QVector<QVariant> > line(_matrix);
    while (line.hasNext())
    {
        int col =0;
        QVectorIterator<QVariant> valC(line.next());
        while (valC.hasNext())
        {
          QVariant val = valC.next();
          ///if( col  >= numberStringColumns )
          ///    valCsv += ValToString(val.toDouble(), 12);
          ///  else
              valCsv += val.toString();
          if(valC.hasNext() )
               valCsv += ",";
          col++;
        }
        valCsv += "\n";
    }

   return valCsv;
}

void TCSVContainer::toggleX( int ncolmn )
{
    int ii = _xcolms.indexOf( ncolmn );
    if( ii != -1 )
       _xcolms.remove( ii );
    else
       if( ncolmn<columnCount() && getType( 0, ncolmn ) == ftDouble )
           _xcolms.append( ncolmn );
}

void TCSVContainer::toggleY( int ncolmn )
{
    int ii = _ycolms.indexOf( ncolmn );
    if( ii != -1 )
       _ycolms.remove( ii );
    else
        if( ncolmn<columnCount() && getType( 0, ncolmn ) == ftDouble )
             _ycolms.append( ncolmn );
}

/// Test empty value (skip point)
bool TCSVContainer::isEmptyValue( const QPointF& ) const
{
    //return ( pos.x() == DOUBLE_EMPTY || pos.y() == DOUBLE_EMPTY );
    return false;
}

/// Get point from one line to paint  (ndxX - column in Abscissa table)
QPointF TCSVContainer::getPoint( int line, int ndx, int ndxX )
{
   double valx=0, valy = data( ndx, _ycolms.at(line) ).toDouble();
   if( ndxX<0 )
     valx = ndx;
   else
     valx = data(ndx, _xcolms.at(ndxX) ).toDouble();
   return QPointF(valx, valy);
}


//--------------------------------------------------------------------

TCSVPage::TCSVPage(const char* apath, QWidget *parent) :
    QWidget(parent), _path(apath), _fname("undef"), _grdata(0), _graph_dlg(0)
{
   // set up window
   _vLayout = new QVBoxLayout(this);
   _vLayout->setObjectName(QStringLiteral("verticalLayout"));

   QString valCsv;
   _data = new TCSVContainer("csv", valCsv);
   _modelCSV = new TMatrixModel( _data, this );
   _tableCSV = new TMatrixTable( this );
   TSortFilterProxyModel *proxyModel = new TSortFilterProxyModel();
   proxyModel->setSourceModel( _modelCSV );
   TMatrixDelegate *deleg = new TMatrixDelegate( this);
    _tableCSV->setItemDelegate(deleg);
    _tableCSV->setModel(proxyModel/*modelFitResults*/);
   _vLayout->addWidget(_tableCSV);

   if( !_path.isEmpty() )
     readCSVFile( apath );
}

TCSVPage::~TCSVPage()
{
    delete _grdata;
    delete _graph_dlg;

    delete _tableCSV;
    delete _vLayout;
    delete _data;
}

void TCSVPage::readCSVFile( const QString& apath )
{
   _path = apath;
    QFileInfo fi(_path);
   _fname = fi.baseName().toUtf8().data();

    // read file
    QFile tmpStriam(_path);
    QString valCsv;
    if(tmpStriam.open( QIODevice::ReadOnly ))
    {
      valCsv = tmpStriam.readAll();
      tmpStriam.close();

      //delete all comments // #
      removeComments( valCsv );
      // remove all ' ' and '\t'
      valCsv.remove(QChar(' '), Qt::CaseInsensitive);
      valCsv.remove(QChar('\t'), Qt::CaseInsensitive);

      //set up data
      _data->setCsvString(valCsv);
      _modelCSV->resetMatrixData();
    }
    else
      std::cout << "error open file " << _path.toUtf8().data() << endl;
 }

void TCSVPage::saveCSVFile( const QString& fpath )
{
    // OR!!! _tableCSV->saveAllToCSV(fpath.toUtf8().data());
    // get string to output
    QString valCsv = _data->getCsvString();
    // write file
    QFile tmpStriam(fpath);
    if(tmpStriam.open( QIODevice::WriteOnly|QIODevice::Truncate))
    {
      tmpStriam.write(valCsv.toUtf8());
      tmpStriam.close();
    }
}
