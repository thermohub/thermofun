#ifndef THERMOMODEL_H
#define THERMOMODEL_H

#include <QObject>
#include "jsonui/model_table.h"
// ThermoFun includes
#ifdef FROM_SRC
#include "../src/DBClient/AbstractData.h"
#else
#include "thermofun/DBClient/AbstractData.h"
#endif

class ThermoViewModel: public QObject
{
    Q_OBJECT

    //QSortFilterProxyModel *proxyModel;
    ///std::shared_ptr<ThermoFun::AbstractData> data__;
    ThermoFun::AbstractData* data_;

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

    void loadModeRecords(const jsonio::ValuesTable&  matr )
    {
      thermoData->updateValues( matr );
      thermoModel->resetMatrixData();
    }

    /// Resize model to selected
    void leftOnlySelected(  const vector<int> selrows )
    {
        jsonio::ValuesTable newmatr;
        const jsonio::ValuesTable&  oldmatr = getValues();

        for( auto rowndx: selrows)
          newmatr.push_back( oldmatr[rowndx] );
        loadModeRecords(newmatr );
    }

    vector<string> getColumn( uint column, const vector<int>& selrows ) const
    {
        vector<string> keys;
        const jsonio::ValuesTable&  matrix = getValues();
        for( auto rowndx: selrows)
        {
          auto row = matrix[rowndx];
          if( column >= row.size())
              keys.push_back( "" );
           else
             keys.push_back( row[column]);
        }
        return keys;
    }

    int findRow( uint column, const string& value ) const
    {
        const jsonio::ValuesTable&  matrix = getValues();
        for( uint ii=0; ii<matrix.size(); ii++ )
        {
          if( column >= matrix[ii].size())
              continue;
           else
             if( matrix[ii][column] == value )
              return ii;
        }
        return -1;
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

    jsonui::TMatrixModel* getModel() const
    {
      return  thermoModel.get();
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
