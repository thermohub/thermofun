#ifndef THERMOMODEL_H
#define THERMOMODEL_H

#include <QObject>
//#include "jsonio/dbquerydef.h"
#include "jsonui/model_table.h"
//#include "ThermoData.h"

// ThermoFun includes
#ifdef FROM_SRC
#include "../src/DBClient/AbstractData.h"
#include "../src/Reaction.h"
#include "../src/Substance.h"
#endif
#ifndef FROM_SRC
#include "thermofun/DBClient/AbstractData.h"
#include "thermofun/Reaction.h"
#include "thermofun/Substance.h"
#endif

using MapLevelReaction             = map<string, ThermoFun::Reaction>;
using MapSymbolMapLevelReaction    = map<string, MapLevelReaction>;

class ThermoViewModel: public QObject
{
    Q_OBJECT

    //QSortFilterProxyModel *proxyModel;
    ThermoFun::AbstractData *data_;

    /// loaded data
    std::shared_ptr<jsonui::StringTable> thermoData;
    /// data model
    std::shared_ptr<jsonui::TMatrixModel>  thermoModel;

public:

    ThermoViewModel( ThermoFun::AbstractData *data, QObject* parent = 0 ):
        QObject(parent), data_(data)
      {
         defineModels();
      }
    virtual ~ThermoViewModel()
    { }

    void linkData( ThermoFun::AbstractData *data )
    {
      data_ = data;
      thermoData->updateHeads( data_->getDataHeaders() );
      thermoData->updateValues( {} );
      thermoModel->resetMatrixData();
    }

    void loadModeRecords( const string& idReactionSet )
    {
      auto matr = data_->loadRecordsValues( idReactionSet );
      thermoData->updateValues( matr );
      thermoModel->resetMatrixData();
    }

    void loadModeRecords( const jsonio::DBQueryData& query, int sourcetdb,
                          const vector<ThermoFun::ElementKey>& elements )
    {
      auto matr = data_->loadRecordsValues(  query, sourcetdb, elements );
      thermoData->updateValues( matr );
      thermoModel->resetMatrixData();
    }

    void loadModeRecords(const vector<vector<string>> matr )
    {
      thermoData->updateValues( matr );
      thermoModel->resetMatrixData();
    }

    const jsonio::DBQueryDef& getQuery( ) const
    {
      return data_->getDB()->getQuery();
    }

    const jsonio::ValuesTable&  getValues() const
    {
      return thermoData->getValues();
    }
    void updateModel( jsonui::TMatrixTable *tableView )
    {
        if( tableView  )
          tableView->setModel(thermoModel.get());
    }

protected:

    // initialization
    void defineModels()
    {
       thermoData.reset( new jsonui::StringTable( "records", data_->getDataHeaders()  ) );
       thermoModel.reset( new jsonui::TMatrixModel( thermoData.get() ) );
    }

    void setHeader( vector<string> heads)
    {
      data_->setDataHeaders(heads);
    }

    void setFields( vector<string> fields)
    {
      return data_->setDataFieldPaths(fields);
    }
};


#endif // THERMOMODEL_H
