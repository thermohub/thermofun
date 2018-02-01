#ifndef THERMOSETDATA_H
#define THERMOSETDATA_H

#include "AbstractData.h"

namespace ThermoFun {

class ThermoSetData : public AbstractData
{
public:

    ThermoSetData( const jsonio::TDataBase* dbconnect );

    /// Construct a copy of an SubstanceData instance
    ThermoSetData(const ThermoSetData& other);

    auto operator=(ThermoSetData other) -> ThermoSetData&;

    virtual ~ThermoSetData();

    /// Extract data connected to ReactionSet
    virtual jsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable  loadRecordsValues( const string& query, int sourcetdb,
                                                    const vector<ElementKey>& elements = {} );
    /// Get Elements list from reaction record
    virtual set<ElementKey> getElementsList( const string& idThermoDataSet);

    auto idRecordFromSymbol (const string &symbol) -> string;


private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};


}

#endif // THERMOSETDATA_H
