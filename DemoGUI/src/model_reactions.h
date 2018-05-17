#ifndef _model_reaction_h
#define _model_reaction_h

#include <memory>
#include <QAbstractItemModel>
#include <QTreeView>
#include <QItemDelegate>
#include "jsonio/ar2base.h"

// ThermoFun includes
#include "thermofun/ThermoProperties.h"

/// Definition of tree reaction line structure
class ReactionTree
{

public:

    ReactionTree( int andx, const string& aname, vector<double>& avalues, vector<double>& aerrors, vector<Reaktoro_::Status>& astatus, ReactionTree* aparent ):
       ndx(andx), rsnames(aname), values(avalues), errors(aerrors), status(astatus), parent(aparent)
    {}

    ReactionTree( int andx, const string& aname, ReactionTree* aparent = 0 ):
        ndx(andx), rsnames(aname), parent(aparent)
    {}

    ~ReactionTree(){}

    int ndx;
    string rsnames;         // name of field
    vector<double> values;  // values of field
    vector<double> errors;  // errors of field
    vector<Reaktoro_::Status> status;     // status of value

    ReactionTree *parent;
    vector<unique_ptr<ReactionTree>> children;

/*    /// Get Value from string
    /// If field is not type T, the false will be returned.
    template <class T>
    bool getValue( int ndx, T& val  )
    {
        if(ndx >= values.size() )
          return false;
        string valstr = values[ndx].toUtf8().data();
        return  jsonio::TArray2Base::string2value( valstr, val );
    }

    /// Set Value to Node
    template <class T>
    void setValue( int ndx, const T& value  )
    {
        if(ndx < values.size() )
        {
          string valstr  =  jsonio::TArray2Base::value2string( value );
          values[ndx] = valstr.c_str();
        }
    }
*/
};


//===========================================
// TReactionModel class
//===========================================


/*!
  \ class TReactionModel
  \ class for represents the data set and is responsible for fetching the data
  \ is needed for viewing and for writing back any changes.
  \ Reading/writing data from/to ReactionLine object
*/
class TReactionModel: public QAbstractItemModel
{
	Q_OBJECT

    QStringList hdData;
    ReactionTree* rootNode;

 public:
	  
  TReactionModel(  ReactionTree* newRoot, QObject* parent = 0 );
  ~TReactionModel(){}
	 
  QModelIndex index(int row, int column, const QModelIndex& parent) const;
  QModelIndex parent(const QModelIndex& child) const;
  int rowCount ( const QModelIndex& parent ) const;     //ok
  int columnCount ( const QModelIndex& parent  ) const; // ok
  QVariant data ( const QModelIndex& index, int role ) const;
  //bool setData ( const QModelIndex& index, const QVariant& value, int role );
  QVariant headerData ( int section, Qt::Orientation orientation, int role ) const; 
  Qt::ItemFlags flags ( const QModelIndex& index ) const;
	 
  ReactionTree* lineFromIndex(const QModelIndex& index) const;

  void setModelData(ReactionTree* newRoot )
  {
      beginResetModel();
      rootNode = newRoot;
      endResetModel();
  }

};


/*!
  \ class TReactionView implements a tree view of reactions structure
  \ that displays items from a model. This class is used to 
  \ provide standard tree views that were previously provided by 
  \ the QTree class, but using the more flexible approach 
  \ provided by Qt's model/view architecture. 
*/
class TReactionView: public QTreeView
{
	Q_OBJECT
	
	void keyPressEvent(QKeyEvent* e);
    QMenu* contextMenu(const QModelIndex &index);

    QString createString();
    QString createHeader();
    void selectWithChildren(const QModelIndex& parIndex);
    void copyWithChildren( const QModelIndex& parIndex,
        const QModelIndexList& selIndexes, QString& clipText );

 protected slots:

    void slotPopupContextMenu(const QPoint& pos);
    void changeCurrent( int section );

 public slots:
    void CmHelp(){}
    void CmSelectRow();
    void CmSelectColumn();
    void CmSelectAll();
    void CmSelectGroup();
    void CmCopyData();
    void CmCopyDataHeader();

 public:

    TReactionView( QWidget * parent = 0 );

};

#endif   // _model_reaction_h
