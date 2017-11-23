#ifndef _SUBSTANCEDATA_H
#define _SUBSTANCEDATA_H

// C++ includes
#include <memory>
#include "formuladata.h"
#include "AbstractData.h"

namespace ThermoFun {

//struct ThermoDataAbstract;

class SubstanceData_ : public AbstractData
{
public:

    SubstanceData_( );

    /// Construct a copy of an SubstanceData instance
    SubstanceData_(const SubstanceData_& other);

    auto operator=(SubstanceData_ other) -> SubstanceData_&;

    virtual ~SubstanceData_();

    /// Extract data connected to ReactionSet
    virtual bsonio::ValuesTable  loadRecordsValues( const string& idReactionSet );
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

    /**
     * @brief querySolvents
     * @param sourcetdb
     * @return
     */
    auto querySolvents(int sourcetdb) -> vector<vector<string>>;

private:

    struct Impl;
    std::shared_ptr<Impl> pimpl;
};

}



#endif // _SUBSTANCEDATA_H
