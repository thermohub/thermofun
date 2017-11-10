#ifndef REACTIONDATA_H
#define REACTIONDATA_H

// C++ includes
#include <memory>
#include "formuladata.h"
#include "ThermoData.h"

namespace ThermoFun
{

// struct ThermoDataAbstract;

class ReactionData : public ThermoDataAbstract
{
  public:
    ReactionData();

    /// Construct a copy of an ReactionData instance
    // ReactionData(const ReactionData& other);

    auto operator=(ReactionData other) -> ReactionData &;

    virtual ~ReactionData();

    /// Extract data connected to ReactionSet
    //  virtual bsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual bsonio::ValuesTable loadRecordsValues(const string &query, int sourcetdb,
                                                  const vector<ElementKey> &elements = {});
    /// Get Elements list from reaction record
    virtual set<ElementKey> getElementsList(const string &id);

    /// Return all formulas from all connected substances
    vector<string> getReactantsFormulas(const string &idReaction);

    /**
     * @brief queryIncomingEdgesTakes
     * @param idReact
     * @param queryFields
     * @return
     */
    auto queryInEdgesTakes(string idReact, vector<string> queryFields) -> vector<string>;

    /**
     * @brief mapReactantsCoeff
     * @param _idReac
     * @return
     */
    auto reactantsCoeff(string idReact) -> std::map<string, double>;

  private:
    bool testElements(const string &idReaction, const vector<ElementKey> &elements);

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};
}

#endif // REACTIONDATA_H
