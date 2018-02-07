#ifndef _REACTIONDATA_H
#define _REACTIONDATA_H

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
    virtual jsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable loadRecordsValues(const jsonio::DBQueryData& query, int sourcetdb,
                                                  const vector<ElementKey> &elements = {});
    /// Get Elements list from reactions
    virtual set<ElementKey> getElementsList(const string &id);

    /// Return all formulas from all connected substances
    vector<string> getReactantsFormulas(const string &idReaction);

    /**
     * @brief queryIncomingEdgesTakes returns data of the incoming edges of type takes
     * @param idReact reaction id
     * @param queryFields which data from record for be extracted in the result
     * @return list of extracted data for each edge
     */
    auto queryInEdgesTakes(string idReact, vector<string> queryFields) -> vector<string>;

    /**
     * @brief mapReactantsCoeff returns the map of reactants symbols and coefficients
     * connected to a reaction with an edge of type takes
     * @param _idReac reaction id
     * @return
     */
    auto reactantsCoeff(string idReact) -> std::map<string, double>;

    /**
     * @brief resetRecordElements resets elements list in record
     * @param idReact
     */
    auto resetRecordElements(const string& idReact) -> void;

    /**
     * @brief checkReactSymbolLevel checks if reaction with the same symbol / level exists. If exists the level is increased and the
     * symbol is annotated with _#level
     * @param sourcetdb
     * @param symbol
     * @param level
     * @return
     */
    bool checkReactSymbolLevel (string sourcetdb, string &symbol, string &level);

  private:

    // tests if elements are present in the reaction
    bool testElements(const string &idReaction, const vector<ElementKey> &elements);

    // returns a list of ids for all reaction with symbol and sourcetdb present in the database
    vector<string> getKeys(string symbol, string sourcetdb);

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};
}

#endif // _REACTIONDATA_H
