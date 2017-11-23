#ifndef _REACTIONDATA_H
#define _REACTIONDATA_H

// C++ includes
#include <memory>
#include "formuladata.h"
#include "AbstractData.h"

namespace ThermoFun
{

// struct ThermoDataAbstract;

class ReactionData_ : public AbstractData
{
  public:
    ReactionData_();

    /// Construct a copy of an ReactionData instance
    ReactionData_(const ReactionData_& other);

    auto operator=(ReactionData_ other) -> ReactionData_ &;

    virtual ~ReactionData_();

    /// Extract data connected to ReactionSet
    virtual bsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual bsonio::ValuesTable loadRecordsValues(const string &query, int sourcetdb,
                                                  const vector<ElementKey> &elements = {});
    /// Get Elements list from reactions
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

    auto resetRecordElements(const string& idReact) -> void;

  private:
    bool testElements(const string &idReaction, const vector<ElementKey> &elements);

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};
}

#endif // _REACTIONDATA_H
