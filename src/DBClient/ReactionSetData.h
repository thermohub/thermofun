#ifndef REACTIONSETDATA_H
#define REACTIONSETDATA_H

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
    virtual jsonio::ValuesTable  loadRecordsValues( const std::string& idReactionSet );
    /// Extract data by condition
    virtual jsonio::ValuesTable loadRecordsValues(const jsonio::DBQueryData& query, int sourcetdb,
                                                  const std::vector<ElementKey> &elements = {});
    /// Extract data connected to ThermoDataSet
    virtual  std::vector<std::string> selectGiven( const std::vector<std::string>& idThermoDataSets, bool unique = true );

    std::vector<std::string> selectGivenSubstances( const std::vector<int>& sourcetdbs,
                       const std::vector<std::string>& substanceSymbols, bool unique = true );

    /// Select reaction sets belonging to sourcetdbs and containing given reaction symbols
    std::vector<std::string> selectGiven( const std::vector<int>& sourcetdbs,
                       const std::vector<std::string>& reactionSymbols, bool unique = true );

    /// Extract data connected to ThermoDataSet and containing given reaction symbols
    std::vector<std::string> selectGiven( const std::string& idThermoDataSet, const std::vector<std::string>& reactionSymbols );

    /// Get Elements list from reactions
    virtual std::set<ElementKey> getElementsList(const std::string &id);

    /// Link to table of fields values loaded before
    const jsonio::ValuesTable& getValuesTable();

    /**
     * @brief resetRecordElements resets elements list in record
     * @param aKey record key
     */
    void resetRecordElements( const std::string& aKey );

    /// Return all ids from all connected substances
    std::vector<std::string> getSubstanceIds( const std::string& idrcset );

    /// Return all formulas from all connected substances
    std::vector<std::string> getSubstanceFormulas( const std::string& idreaction );

    /// Read species map from record
    bool getSpeciesMap( const std::string& RcSid, std::map<std::string, int>& specmap );

  private:


    /// Test record all elements exist into list
    bool testElements( const std::string& idrecord,
                       const std::vector<ElementKey>& elements );

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};

}

#endif // REACTIONSETDATA_H
