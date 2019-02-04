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
    virtual jsonio::ValuesTable  loadRecordsValues( const std::string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable  loadRecordsValues( const jsonio::DBQueryData& query, int sourcetdb,
                                                    const std::vector<ElementKey>& elements = {} );
    /// Extract data connected to ThermoDataSet
    virtual std::vector<std::string> selectGiven( const std::vector<std::string>& idThermoDataSets, bool unique = true );

    /// Get Elements list from reaction record
    virtual std::set<ElementKey> getElementsList( const std::string& idThermoDataSet);

    /// Link to table of fields values loaded before
    const jsonio::ValuesTable& getValuesTable();

    /// Return all formulas from all connected substances
    std::vector<std::string> getSubstanceFormulas( const std::string& idThermoDataSet );


    auto idRecordFromSymbol (const std::string &symbol) -> std::string;

    /// Visit  all linked Vertexes
    void traverceVertexes( const std::string& idThermoDataSet, jsonio::GraphElementFunction afunc );
    /// Visit  all linked Edges
    void traverceEdges( const std::string& idThermoDataSet, jsonio::GraphElementFunction afunc );

private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};


}

#endif // THERMOSETDATA_H
