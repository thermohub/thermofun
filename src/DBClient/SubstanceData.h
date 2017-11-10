#ifndef SUBSTANCEDATA_H
#define SUBSTANCEDATA_H

// C++ includes
#include <memory>
#include "formuladata.h"
#include "ThermoData.h"

namespace ThermoFun {

//struct ThermoDataAbstract;

class SubstanceData : public ThermoDataAbstract
{
public:

    SubstanceData( );

    auto operator=(SubstanceData other) -> SubstanceData&;

    virtual ~SubstanceData();

    /// Extract data connected to ReactionSet
//    virtual bsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
    /// Extract data by condition
    virtual bsonio::ValuesTable  loadRecordsValues( const string& query, int sourcetdb,
                                                    const vector<ElementKey>& elements = {} );
    /// Get Elements list from reaction record
    virtual set<ElementKey> getElementsList( const string& idSubstance );

    /**
     * @brief queryIncomingEdgesDefines
     * @param idSubst
     * @param queryFields
     * @param level
     * @return
     */
    auto queryInEdgesDefines(string idSubst, vector<string> queryFields,  string level) -> vector<string>;

    /**
     * @brief definesReactionSymbol
     * @param idSubst
     * @param level
     * @return
     */
    auto definesReactionSymbol(string idSubst, string level) -> std::string;

private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};

}



#endif // SUBSTANCEDATA_H
