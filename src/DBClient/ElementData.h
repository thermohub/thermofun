#ifndef ELEMENTDATA_H
#define ELEMENTDATA_H

#include "AbstractData.h"

namespace ThermoFun
{

class ElementData_ : public AbstractData
{
  public:
    ElementData_(const jsonio::TDataBase* dbconnect);

    /// Construct a copy of an ReactionData instance
    ElementData_(const ElementData_& other);

    auto operator=(ElementData_ other) -> ElementData_ &;

    virtual ~ElementData_();

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


  private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};

}

#endif // ELEMENTDATA_H
