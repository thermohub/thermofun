#ifndef SUBSTANCEDATA_H
#define SUBSTANCEDATA_H

// C++ includes
#include <memory>
#include "AbstractData.h"

namespace ThermoFun {

//struct ThermoDataAbstract;
using MapLevel_IdReaction                   = std::map<std::string, std::string>;
using MapSubstSymbol_MapLevel_IdReaction    = std::map<std::string, MapLevel_IdReaction>;

class SubstanceData_ : public AbstractData
{
public:

    SubstanceData_( const jsonio::TDataBase* dbconnect );

    /// Construct a copy of an SubstanceData instance
    SubstanceData_(const SubstanceData_& other);

    auto operator=(SubstanceData_ other) -> SubstanceData_&;

    virtual ~SubstanceData_();

    /// Extract data connected to ReactionSet
    virtual jsonio::ValuesTable  loadRecordsValues( const std::string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable  loadRecordsValues( const jsonio::DBQueryData& query, int sourcetdb,
                                                    const std::vector<ElementKey>& elements = {} );
    /// Extract data connected to ThermoDataSet
    virtual std::vector<std::string> selectGiven( const std::vector<std::string>& idThermoDataSets, bool unique = true );
    std::vector<std::string> selectGiven( const std::vector<int>& sourcetdbs, const std::vector<ElementKey>& elements, bool unique = true );
    std::vector<std::string> selectGiven( const std::string& idThermoDataSet,
                       const std::vector<ElementKey>& elements, bool unique = true );
    /// Get Elements list from reaction record
    virtual std::set<ElementKey> getElementsList( const std::string& idSubstance );

     /// Link to table of fields values loaded before
     const jsonio::ValuesTable& getValuesTable();

    /**
     * @brief queryIncomingEdgesDefines query the ids of incoming edges of type defines
     * @param idSubst substance id
     * @param level level of the edge
     * @return list of ids
     */
    auto queryInEdgesDefines(std::string idSubst,  std::string level) -> std::vector<std::string>;

    /**
     * @brief definesReactionSymbol returns the reaction symbol which defines a substance
     * @param idSubst substance id
     * @param level edge defines level
     * @return reaction symbol
     */
    auto definesReactionSymbol(std::string idSubst, std::string level) -> std::string;

    /**
     * @brief querySolvents returns the solvents data
     * @param sourcetdb
     * @return solvents data determind by the dataFieldPaths of the object
     * { "properties.symbol","properties.name","properties.formula","_id", "properties.class_", "properties.sourcetdb"};
     */
    auto querySolvents(int sourcetdb) -> std::vector<std::vector<std::string>>;

    /**
     * @brief getSubstanceLevel returns the level which is set for the substance
     * @param substSymbol substance symbol
     * @return level value
     */
    auto getSubstanceLevel(std::string substSymbol) const -> std::string;

    /**
     * @brief setSubstanceLevel
     * @param substSymbol
     * @param level value
     */
    auto setSubstanceLevel(std::string substSymbol, std::string level) -> void;

    /**
     * @brief nextValueForDefinesLevel looks through the database for all eges defines incoming towards the substance
     * and returns the maximum level+1, found for the defines edges
     * @param idSubst substance id
     * @return level value
     */
    auto nextValueForDefinesLevel (std::string idSubst) const -> std::string;

    /**
     * @brief recordsMapLevelDefinesReaction returns the map of substances and map of levels and id reactions
     * e.g. CaCO3@ can be defined on level one by the aragonite solubility reaction and on level 2 by the
     * calcite solubility reaction
     * @return
     */
    MapSubstSymbol_MapLevel_IdReaction recordsMapLevelDefinesReaction( );

    /**
     * @brief recordsMapLevelDefinesReaction returns the map of substances and map of levels and id reactions, for a given
     * list of substances
     * e.g. CaCO3@ can be defined on level one by the aragonite solubility reaction and on level 2 by the
     * calcite solubility reaction
     * @param connectedSubstIds list of substances ids
     * @param connectedSubstSymbols list of substances symbols
     * @return
     */
    MapSubstSymbol_MapLevel_IdReaction recordsMapLevelDefinesReaction(std::vector<std::string> connectedSubstIds, std::vector<std::string> connectedSubstSymbols );



private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;
    void updateTableByElementsList( jsonio::ValuesTable& substQueryMatr, const std::vector<ElementKey>& elements );
};

}



#endif // SUBSTANCEDATA_H
