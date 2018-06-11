#ifndef _SUBSTANCEDATA_H
#define _SUBSTANCEDATA_H

// C++ includes
#include <memory>
#include "AbstractData.h"

namespace ThermoFun {

//struct ThermoDataAbstract;
using MapLevel_IdReaction                   = map<string, string>;
using MapSubstSymbol_MapLevel_IdReaction    = map<string, MapLevel_IdReaction>;

class SubstanceData_ : public AbstractData
{
public:

    SubstanceData_( const jsonio::TDataBase* dbconnect );

    /// Construct a copy of an SubstanceData instance
    SubstanceData_(const SubstanceData_& other);

    auto operator=(SubstanceData_ other) -> SubstanceData_&;

    virtual ~SubstanceData_();

    /// Extract data connected to ReactionSet
    virtual jsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable  loadRecordsValues( const jsonio::DBQueryData& query, int sourcetdb,
                                                    const vector<ElementKey>& elements = {} );
    /// Extract data connected to ThermoDataSet
    virtual vector<string> selectGiven( const vector<string>& idThermoDataSets, bool unique = true );
    vector<string> selectGiven( const vector<int>& sourcetdbs, const vector<ElementKey>& elements, bool unique = true );

    /// Get Elements list from reaction record
    virtual set<ElementKey> getElementsList( const string& idSubstance );

    /**
     * @brief queryIncomingEdgesDefines query the ids of incoming edges of type defines
     * @param idSubst substance id
     * @param queryFields fields data to be retrieved from the query
     * @param level level of the edge
     * @return list of ids
     */
    auto queryInEdgesDefines(string idSubst, vector<string> queryFields,  string level) -> vector<string>;

    /**
     * @brief definesReactionSymbol returns the reaction symbol which defines a substance
     * @param idSubst substance id
     * @param level edge defines level
     * @return reaction symbol
     */
    auto definesReactionSymbol(string idSubst, string level) -> std::string;

    /**
     * @brief querySolvents returns the solvents data
     * @param sourcetdb
     * @return solvents data determind by the dataFieldPaths of the object
     * { "properties.symbol","properties.name","properties.formula","_id", "properties.class_", "properties.sourcetdb"};
     */
    auto querySolvents(int sourcetdb) -> vector<vector<string>>;

    /**
     * @brief getSubstanceLevel returns the level which is set for the substance
     * @param substSymbol substance symbol
     * @return level value
     */
    auto getSubstanceLevel(string substSymbol) const -> string;

    /**
     * @brief setSubstanceLevel
     * @param substSymbol
     * @param level value
     */
    auto setSubstanceLevel(string substSymbol, string level) -> void;

    /**
     * @brief nextValueForDefinesLevel looks through the database for all eges defines incoming towards the substance
     * and returns the maximum level+1, found for the defines edges
     * @param idSubst substance id
     * @return level value
     */
    auto nextValueForDefinesLevel (string idSubst) const -> string;

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
    MapSubstSymbol_MapLevel_IdReaction recordsMapLevelDefinesReaction(vector<string> connectedSubstIds, vector<string> connectedSubstSymbols );



private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;
    void updateTableByElementsList( jsonio::ValuesTable& substQueryMatr, const vector<ElementKey>& elements );
};

}



#endif // _SUBSTANCEDATA_H
