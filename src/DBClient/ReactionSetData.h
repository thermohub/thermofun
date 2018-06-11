#ifndef _REACTIONSETDATA_H
#define _REACTIONSETDATA_H

#include "AbstractData.h"

namespace ThermoFun
{

class ReactionSetData_ : public AbstractData
{
  public:
    ReactionSetData_(const jsonio::TDataBase* dbconnect);

    /// Construct a copy of an ReactionData instance
    ReactionSetData_(const ReactionSetData_& other);

    auto operator=(ReactionSetData_ other) -> ReactionSetData_ &;

    virtual ~ReactionSetData_();

    /// Extract data connected to ReactionSet
    virtual jsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable loadRecordsValues(const jsonio::DBQueryData& query, int sourcetdb,
                                                  const vector<ElementKey> &elements = {});
    /// Extract data connected to ThermoDataSet
    virtual  vector<string> selectGiven( const vector<string>& idThermoDataSets, bool unique = true );
    vector<string> selectGiven( const vector<int>& sourcetdbs,
                       const vector<string>& substanceSymbols, bool unique );

    /// Get Elements list from reactions
    virtual set<ElementKey> getElementsList(const string &id);

    /**
     * @brief resetRecordElements resets elements list in record
     * @param aKey record key
     */
    void resetRecordElements( const string& aKey );

    /// Return all ids from all connected substances
    vector<string> getSubstanceIds( const string& idrcset );

    /// Return all formulas from all connected substances
    vector<string> getSubstanceFormulas( const string& idreaction );

    /// Read species map from record
    bool getSpeciesMap( const string& RcSid, std::map<string, int>& specmap );

  private:


    /// Test record all elements exist into list
    bool testElements( const string& idrecord,
                       const vector<ElementKey>& elements );

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};

}

#endif // _REACTIONSETDATA_H
