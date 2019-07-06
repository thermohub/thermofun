#ifndef REACTIONDATA_H
#define REACTIONDATA_H

// C++ includes
#include <memory>
#include "AbstractData.h"

namespace ThermoFun
{

class ReactionData_ : public AbstractData
{
  public:
    ReactionData_(const jsonio::TDataBase* dbconnect);

    /// Construct a copy of an ReactionData instance
    ReactionData_(const ReactionData_& other);

    auto operator=(ReactionData_ other) -> ReactionData_ &;

    virtual ~ReactionData_();

    /// Extract data connected to ReactionSet
    virtual jsonio::ValuesTable  loadRecordsValues( const std::string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable loadRecordsValues(const jsonio::DBQueryData& query, int sourcetdb,
                                                  const std::vector<ElementKey> &elements = {});
    /// Extract data connected to ThermoDataSet
    virtual  std::vector<std::string> selectGiven( const std::vector<std::string>& idThermoDataSets, bool unique = true );

    /// Select reactions belonging to sourcetdbs and containing given substance symbols
    std::vector<std::string> selectGiven( const std::vector<int>& sourcetdbs,
                       const std::vector<std::string>& substanceSymbols, bool unique=true );
    /// Extract data connected to ThermoDataSet and containing given substance symbols
    std::vector<std::string> selectGiven( const std::string& idThermoDataSet, const std::vector<std::string>& substanceSymbols );

    /// Get Elements list from reactions
    virtual std::set<ElementKey> getElementsList(const std::string &id);

    /// Link to table of fields values loaded before
    const jsonio::ValuesTable& getValuesTable();

    /// Return all formulas from all connected substances
    std::vector<std::string> getReactantsFormulas(const std::string &idReaction);

    /**
     * @brief queryIncomingEdgesTakes returns data of the incoming edges of type takes
     * @param idReact reaction id
     * @return list of extracted data for each edge
     */
    auto queryInEdgesTakes(std::string idReact) -> std::vector<std::string>;

    /**
     * @brief mapReactantsCoeff returns the map of reactants symbols and coefficients
     * connected to a reaction with an edge of type takes
     * @param _idReac reaction id
     * @return
     */
    auto reactantsCoeff(std::string idReact) -> std::map<std::string, double>;

    /**
     * @brief resetRecordElements resets elements list in record
     * @param idReact
     */
    auto resetRecordElements(const std::string& idReact) -> void;

    /**
     * @brief checkReactSymbolLevel checks if reaction with the same symbol / level exists. If exists the level is increased and the
     * symbol is annotated with _#level
     * @param sourcetdb
     * @param symbol
     * @param level
     * @return
     */
    bool checkReactSymbolLevel (int sourcetdb, std::string &symbol, std::string &level);


  private:

    // tests if elements are present in the reaction
    bool testElements(const std::string &idReaction, const std::vector<ElementKey> &elements);

    // returns a list of ids for all reaction with symbol and sourcetdb present in the database
    std::vector<std::string> getKeys(std::string symbol, int sourcetdb);

    void updateTableByElementsList( jsonio::ValuesTable& reactQueryMatr, const std::vector<ElementKey>& elements );

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};
}

#endif // REACTIONDATA_H
