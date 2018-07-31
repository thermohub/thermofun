#ifndef THERMOSETDATA_H
#define THERMOSETDATA_H

#include "AbstractData.h"
#include "jsonio/traversal.h"

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
    virtual jsonio::ValuesTable  loadRecordsValues( const jsonio::DBQueryData& query, int sourcetdb,
                                                    const vector<ElementKey>& elements = {} );
    /// Extract data connected to ThermoDataSet
    virtual  vector<string> selectGiven( const vector<string>& idThermoDataSets, bool unique = true );

    /// Get Elements list from reaction record
    virtual set<ElementKey> getElementsList( const string& idThermoDataSet);

    /// Return all formulas from all connected substances
    vector<string> getSubstanceFormulas( const string& idThermoDataSet );


    auto idRecordFromSymbol (const string &symbol) -> string;

    /// Visit  all linked Vertexes
    void traverceVertexes( const string& idThermoDataSet, jsonio::GraphElementFunction afunc );
    /// Visit  all linked Edges
    void traverceEdges( const string& idThermoDataSet, jsonio::GraphElementFunction afunc );

private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};


}

#endif // THERMOSETDATA_H
