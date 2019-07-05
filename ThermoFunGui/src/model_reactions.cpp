#include <iostream>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QApplication>
#include <QKeyEvent>
#include "model_reactions.h"
#include "jsonui/model_view.h"

using namespace jsonio;

int numDigits = 3;

double toPrecision(double num, int n) {
    if(num == 0)
    {
        return 0.0;
    }
    std::ostringstream oss;
    oss.precision(n);
    oss<<std::fixed;
    oss<<num;
    double result = atof((oss.str()).c_str());
    return  result;
}

TReactionModel::TReactionModel( ReactionTree* newRoot, QObject* parent ):
      QAbstractItemModel(parent)
{
  // define edit tree view
  hdData << "Reactions/Substances" << "Coeff" << "ISOC" << "logKr" << "dGr/G0" << "dHr/H0" << "dSr/S0" << "dCpr/Cp0" << "dVr/V0"  ;
  rootNode = newRoot;
}

ReactionTree *TReactionModel::lineFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<ReactionTree *>(index.internalPointer());
    } else {
        return rootNode;
    }
}


QModelIndex TReactionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!rootNode)
        return QModelIndex();
    ReactionTree *parentItem = lineFromIndex( parent );
    if(parentItem->children.size() > 0 )
       return createIndex(row, column, parentItem->children[row].get());
    else
       return QModelIndex();
}

QModelIndex TReactionModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    ReactionTree *childItem = lineFromIndex(child);
    ReactionTree *parentItem = childItem->parent;
    if (parentItem == rootNode )
        return QModelIndex();
    return createIndex(parentItem->ndx, 0, parentItem);
}


int TReactionModel::rowCount( const QModelIndex& parent ) const
{
   if (!rootNode)
       return 0;
   if (parent.column() > 0)
      return 0;
   ReactionTree *parentItem = lineFromIndex( parent );
   return parentItem->children.size();
}	

int TReactionModel::columnCount( const QModelIndex&  ) const
{
    return hdData.size();
}

Qt::ItemFlags TReactionModel::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  return (flags & ~Qt::ItemIsEditable);
}

QVariant TReactionModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
 if( role == Qt::DisplayRole  && orientation == Qt::Horizontal )
	   return hdData[section];
  return QVariant();
}

QVariant TReactionModel::data( const QModelIndex& index, int role ) const
{
   if(!index.isValid())
	 return QVariant();	

   switch( role )
   { case Qt::DisplayRole:
     case Qt::EditRole:
            {  ReactionTree *item = lineFromIndex( index );
               if( index.column() == 0 )
                   return item->rsnames.c_str();
               else
                   if ( index.column() == 2 ) // ISOC
                   {
                      if  (item->values[index.column()-1] == 1)
                          return "+";
                   }
               else
                   if( index.column() < item->values.size()+1 )
                   {
                       if ((item->values[index.column()-1] != 0) && (index.column()<4))
                           return toPrecision(item->values[index.column()-1], numDigits);
                       if (((item->status[index.column()-1] == Reaktoro_::Status::calculated) || (item->status[index.column()-1] == Reaktoro_::Status::read)
                            || (item->status[index.column()-1] == Reaktoro_::Status::assigned))
                               && (index.column()>3))
                           return toPrecision(item->values[index.column()-1], numDigits);
                   }
           }
      default: break;
   }
   return QVariant();
}

//-------------------------------------------------------------------------------------
// class TReactionView implements a tree view that displays items from a model to reactions.
//-------------------------------------------------------------------------------------

TReactionView::TReactionView( QWidget * parent ):
     QTreeView( parent )
{
     setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
     header()->setSectionsClickable(true);
     header()->setSectionResizeMode( /*QHeaderView::ResizeToContents*/QHeaderView::Interactive );

     //setEditTriggers( QAbstractItemView::DoubleClicked|QAbstractItemView::AnyKeyPressed );
     setFocusPolicy(Qt::StrongFocus);
     //setTabKeyNavigation( false );
     setContextMenuPolicy(Qt::CustomContextMenu);
     setSelectionMode( QAbstractItemView::ContiguousSelection );
     //setSelectionBehavior( QAbstractItemView::SelectRows );
     setIndentation( 30 );

     connect( this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotPopupContextMenu(QPoint)));
     connect( header(), SIGNAL(sectionClicked(int)),
             this, SLOT(changeCurrent(int)));
}

void TReactionView::changeCurrent( int section )
{
    if( !currentIndex().isValid() )
    { QModelIndex index = model()->index( 0, section, rootIndex());
      setCurrentIndex(index);
    }
}

void TReactionView::slotPopupContextMenu(const QPoint &pos)
{
    QModelIndex index = indexAt( pos );
    QMenu *menu = contextMenu( index);
    if( menu)
    { menu->exec( viewport()->mapToGlobal(pos) );
      delete menu;
    }
}

QMenu* TReactionView::contextMenu(const QModelIndex& /*index*/)
{
    const TReactionModel* model_ =dynamic_cast<const TReactionModel*>(model());
    if( !model_ )
        return nullptr;
    QMenu *menu = new QMenu(this);
    QAction* act;

    act =  new QAction(tr("&Help"), this);
    act->setIcon(QIcon(":/menu/ShowHelpWindowIcon.png"));
    act->setShortcut(tr("F1"));
    act->setStatusTip(tr("Help for specified cell"));
    connect(act, SIGNAL(triggered()), this, SLOT(CmHelp()));
    menu->addAction(act);

    menu->addSeparator();

    act =  new QAction(tr("Select &row"), this);
    act->setShortcut(tr("Ctrl+R"));
    act->setStatusTip(tr("Select current row"));
    connect(act, SIGNAL(triggered()), this, SLOT(CmSelectRow()));
    menu->addAction(act);

    act =  new QAction(tr("Select co&lumn"), this);
    act->setShortcut(tr("Ctrl+L"));
    act->setStatusTip(tr("Select current column"));
    connect(act, SIGNAL(triggered()), this, SLOT(CmSelectColumn()));
    menu->addAction(act);

    act =  new QAction(tr("Select &group"), this);
    act->setShortcut(tr("Ctrl+G"));
    act->setStatusTip(tr("Select current item with children"));
    connect(act, SIGNAL(triggered()), this, SLOT(CmSelectGroup()));
    menu->addAction(act);

    act =  new QAction(tr("Select &all"), this);
    act->setShortcut(tr("Ctrl+A"));
    act->setStatusTip(tr("Select all"));
    connect(act, SIGNAL(triggered()), this, SLOT(CmSelectAll()));
    menu->addAction(act);

    menu->addSeparator();

    act =  new QAction(tr("&Copy"), this);
    act->setShortcut(tr("Ctrl+C"));
    act->setStatusTip(tr("Copy selected data"));
    connect(act, SIGNAL(triggered()), this, SLOT(CmCopyData()));
    menu->addAction(act);

    act =  new QAction(tr("Copy with &header"), this);
    act->setShortcut(tr("Ctrl+H"));
    act->setStatusTip(tr("Copy selected header&cells"));
    connect(act, SIGNAL(triggered()), this, SLOT(CmCopyDataHeader()));
    menu->addAction(act);

    return menu;
 }

void TReactionView::keyPressEvent(QKeyEvent* e)
{
  if ( e->modifiers() & Qt::ControlModifier )
  {
   switch ( e->key() )
   {
    case Qt::Key_R:
       CmSelectRow();
       return;
    case Qt::Key_L:
       CmSelectColumn();
       return;
    case Qt::Key_G:
       CmSelectGroup();
       return;
    case Qt::Key_A:
       CmSelectAll();
       return;
    case Qt::Key_C:
       CmCopyData();
      return;
    case Qt::Key_H:
      CmCopyDataHeader();
      return;
   }
 }

  switch( e->key() )
  {
    case Qt::Key_F1:
       CmHelp();
       return;
   }
  QTreeView::keyPressEvent(e);
}

void TReactionView::CmSelectRow()
{
    selectionModel()->select( currentIndex(),
          QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect );
}

void TReactionView::CmSelectColumn()
{
    selectionModel()->select( currentIndex(),
            QItemSelectionModel::Columns|QItemSelectionModel::ClearAndSelect );
}

void TReactionView::CmSelectAll()
{
   selectWithChildren(rootIndex());
}

void TReactionView::selectWithChildren(const QModelIndex& parIndex)
{
   selectionModel()->select( parIndex,
           QItemSelectionModel::Rows|QItemSelectionModel::Select );

   for(int row = 0; row < model()->rowCount( parIndex ); row++ )
   {
      QModelIndex  childIndex = model()->index( row, 0, parIndex);
      selectWithChildren(childIndex);
   }
}

void TReactionView::CmSelectGroup()
{
   if( !currentIndex().isValid() )
      return;
   selectWithChildren(currentIndex());
}


void TReactionView::copyWithChildren( const QModelIndex& parIndex,
    const QModelIndexList& selIndexes, QString& clipText )
{
    QModelIndex index;
    QString cText;
    bool frst = true;
    int row = parIndex.row(),  col;

    for (col = 0; col < model()->columnCount( parIndex ); col++ )
    {
        index = parIndex.sibling( row, col);
        if( selIndexes.contains( index ) )
        {
          if( !frst )
             clipText += jsonui::splitCol;
           frst = false;
           cText = model()->data(index).toString();
           if( cText == emptiness.c_str() )
                 cText = "  ";//"\r";
           clipText += cText;
        }
   }
   if( !frst )
     clipText += jsonui::splitRow;

   for(int rw = 0; rw < model()->rowCount( parIndex ); rw++ )
   {
      index = model()->index( rw, 0, parIndex);
      copyWithChildren( index, selIndexes, clipText );
   }
}

QString TReactionView::createString()
{
  QString clipText;
  QModelIndexList selIndexes = selectedIndexes();
  copyWithChildren( rootIndex(),selIndexes, clipText );
  return clipText;
}

QString TReactionView::createHeader()
{
  QString cText;
  QString clipText;
  int col;
  bool frst = true;
  for( col = 0; col < model()->columnCount( rootIndex() ); col++ )
  {
      if( selectionModel()->columnIntersectsSelection( col,  rootIndex() ) )
      {
        if( !frst )
          clipText += jsonui::splitCol;
        frst = false;
        cText = model()->headerData( col, Qt::Horizontal, Qt::DisplayRole ).toString();
        if( cText == emptiness.c_str() )
            cText = "  ";//"\r";
        clipText += cText;
      }
   }
   if( !frst )
      clipText += jsonui::splitRow;
  return clipText;
}

void TReactionView::CmCopyData()
{
  QString clipText = createString();
  QApplication::clipboard()->setText(clipText/*, QClipboard::Clipboard*/);
}

void TReactionView::CmCopyDataHeader()
{
  QString clipText = createHeader();
  clipText += createString();
  QApplication::clipboard()->setText(clipText/*, QClipboard::Clipboard*/);
}



//---------------------- End of file  model_reactions.cpp ---------------------------
