#ifndef THERMOMODEL_H
#define THERMOMODEL_H

#include <QObject>
#include "jsonui/model_table.h"
// ThermoFun includes
#ifdef FROM_SRC
#include "../ThermoHubClient/AbstractData.h"
#else
#include "ThermoHubClient/AbstractData.h"
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

    ThermoViewModel( ThermoFun::AbstractData *data, QObject* parent = nullptr ):
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

    void loadModeRecords( const std::string& idReactionSet )
    {
      auto matr = data_->loadRecordsValues( idReactionSet );
      thermoData->updateValues( matr );
      thermoModel->resetMatrixData();
    }

    void loadModeRecords( const jsonio::DBQueryData& query, int sourcetdb,
                          const std::vector<ThermoFun::ElementKey>& elements )
    {
      auto matr = data_->loadRecordsValues(  query, sourcetdb, elements );
      thermoData->updateValues( matr );
      thermoModel->resetMatrixData();
    }

    void loadModeRecords( const std::vector<std::string>& ids )
    {
        auto matr = data_->loadRecords(  ids );
        thermoData->updateValues( matr );
        thermoModel->resetMatrixData();
    }

    void loadModeRecords(const jsonio::ValuesTable&  matr )
    {
      thermoData->updateValues( matr );
      thermoModel->resetMatrixData();
    }

    /// Resize model to selected
    void leftOnlySelected(  const std::set<size_t> selrows )
    {
        jsonio::ValuesTable newmatr;
        const jsonio::ValuesTable&  oldmatr = getValues();

        for( auto rowndx: selrows)
          newmatr.push_back( oldmatr[rowndx] );
        loadModeRecords(newmatr );
    }

    std::vector<std::string> getColumn( size_t column, const std::set<size_t>& selrows ) const
    {
        std::vector<std::string> keys;
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

    std::vector<std::string> getColumn( size_t column ) const
    {
       return thermoData->getColumn(column);
    }

    template< class T >
    void fillColumn( size_t column, const T& ndxs, const std::string& value )
    {
        for( size_t ii=0; ii<ndxs.size(); ii++)
          if( ndxs[ii] < thermoData->rowCount() )
              thermoData->setData( ndxs[ii], column, value.c_str() );
    }

    void fillColumn( size_t column, const std::string& value )
    {
        for( int ii=0; ii< thermoData->rowCount(); ii++)
            thermoData->setData( ii, column, value.c_str() );
    }

    size_t findRow( size_t column, const std::string& value, bool first = false ) const
    {
        const jsonio::ValuesTable&  matrix = getValues();
        for( size_t ii=0; ii<matrix.size(); ii++ )
        {
          if( column >= matrix[ii].size())
              continue;
           else
             if( matrix[ii][column] == value )
              return ii;
        }
        jsonio::jsonioErrIf( !first,  "ThermoViewModel",  "Value " + value + " not exists" );
        return 0;
    }

    /// Move record with values up
    void moveUpByOrder(  size_t column, const std::set<std::string>& values )
    {
        jsonio::ValuesTable newmatr;
        const jsonio::ValuesTable&  matrix = getValues();

        // copy lines from values
        for( auto val: values)
         for( auto row: matrix )
            if( row[column] == val  )
              newmatr.push_back( row );

        // copy other record
        for( auto row: matrix )
          if( values.find( row[column]) == values.end() )
              newmatr.push_back( row );

        loadModeRecords(std::move( newmatr ) );
    }

    /// Gen indexes of record with value into column
    std::vector<size_t> recordToValue( size_t column, const std::string& value ) const
    {
      std::vector<size_t> rowndxs;
      const jsonio::ValuesTable&  matrix = getValues();
      for(size_t ii=0; ii<matrix.size(); ii++ )
          if(  matrix[ii][column] == value )
                rowndxs.push_back(ii);
      return rowndxs;
    }


    /// Gen indexes of record with value into column
    std::vector<size_t> recordToValues( size_t column, const std::vector<std::string>& values ) const
    {
      std::vector<size_t> rowndxs;
      const jsonio::ValuesTable&  matrix = getValues();
      for( size_t ii=0; ii<matrix.size(); ii++ )
      {
         for( auto val: values )
            if(  matrix[ii][column] == val )
            {    rowndxs.push_back(ii);
                 break;
            }
      }
      return rowndxs;
    }


    /*const jsonio::DBQueryDef& getQuery( ) const
    {
      return data_->getDB()->getQuery();
    }*/

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

    void setHeader( std::vector<std::string> heads)
    {
      data_->setDataHeaders(heads);
    }

    void setFields( std::vector<std::string> fields)
    {
      return data_->setDataFieldPaths(fields);
    }
};


#endif // THERMOMODEL_H
