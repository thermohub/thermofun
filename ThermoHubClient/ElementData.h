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

    /// Extract data defined sourcetdbs and elements
    std::vector<ElementKey> selectGiven( const std::vector<int>& sourcetdbs, const std::vector<ElementKey>& elements );

    /// Get Elements list from reactions
    virtual std::set<ElementKey> getElementsList(const std::string &id);

    /// Link to table of fields values loaded before
    const jsonio::ValuesTable& getValuesTable();

  private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;

    void updateTableByElementsList( jsonio::ValuesTable& dataQueryMatr, const std::vector<ElementKey>& elements );
};

auto fromElementNode( const jsonio::JsonDom *object ) -> ElementKey;
bool ElementsKeysFromJson( const std::string elmsjson, std::set<ElementKey>& elements );
auto index_from_map(std::string map) -> int;
bool ElementsKeysFromJsonDomArray( const std::string& keypath, const jsonio::JsonDom *object, std::set<ElementKey>& elements );
auto ElementsKeysToJson( const std::set<ElementKey>& elements ) -> std::string;
void setDBElements( jsonio::TDBVertexDocument* elementDB,
                             const jsonio::DBQueryData& query =  jsonio::DBQueryData("{\"_label\": \"element\" }",jsonio::DBQueryData::qTemplate ) );
auto getElementKeys( jsonio::TDBVertexDocument* elementDB, const std::vector<std::string>& idList ) -> std::vector<ElementKey>;
}

#endif // ELEMENTDATA_H
