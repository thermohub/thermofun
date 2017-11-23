#ifndef _REACTIONSETDATA_H
#define _REACTIONSETDATA_H

#include "formuladata.h"
#include "AbstractData.h"

namespace ThermoFun
{

class ReactionSetData_ : public AbstractData
{
  public:
    ReactionSetData_();

    /// Construct a copy of an ReactionData instance
    ReactionSetData_(const ReactionSetData_& other);

    auto operator=(ReactionSetData_ other) -> ReactionSetData_ &;

    virtual ~ReactionSetData_();

    /// Extract data connected to ReactionSet
    virtual bsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual bsonio::ValuesTable loadRecordsValues(const string &query, int sourcetdb,
                                                  const vector<ElementKey> &elements = {});
    /// Get Elements list from reactions
    virtual set<ElementKey> getElementsList(const string &id);

    void resetRecordElements( const string& aKey );

  private:

    /// Return all formulas from all connected substances
    vector<string> getSubstanceFormulas( const string& idreaction );

    /// Test record all elements exist into list
    bool testElements( const string& idrecord,
                       const vector<ElementKey>& elements );

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};

}

#endif // _REACTIONSETDATA_H
