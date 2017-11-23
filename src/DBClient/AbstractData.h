#ifndef ABSTRACTDATA_H
#define ABSTRACTDATA_H

// C++ includes
#include <memory>

// ThermoFun includes
#include "formuladata.h"

namespace ThermoFun {

class AbstractData
{
public:

    AbstractData( const string &name, const string &query, const vector<string> &paths, const vector<string> &headers, const vector<string> &names );

    /// Construct a copy of an ThermoDataAbstract instance
    AbstractData(const AbstractData& other);

    /// Destroy this instance
    virtual ~AbstractData();

    // load values function
    /// Extract data connected to ReactionSet
    virtual bsonio::ValuesTable  loadRecordsValues( const string& idReactionSet ) = 0;
    /// Extract data by condition
    virtual bsonio::ValuesTable  loadRecordsValues( const string& query, int sourcetdb,
                                                    const vector<ElementKey>& elements = {} ) = 0;
    /// Get Elements list from record
    virtual set<ElementKey> getElementsList( const string& idrec ) = 0;

    auto updateDBClient() -> void;

    auto getDB() const -> boost::shared_ptr<bsonio::TDBGraph>;
    auto getName() const -> string;
    auto getQuery() const -> string;
    auto getDataNames() const -> vector<string>;
    auto getDataHeaders() const -> vector<string>;
    auto getDataFieldPaths() const -> vector<string>;
    auto getDataName_DataIndex() const -> std::map<std::string, int>;
    auto getDataName_DataFieldPath() const -> std::map<std::string, std::string>;
    auto getSubstSymbol_DefinesLevel() const -> std::map<std::string, std::string>;

    auto setDB(const boost::shared_ptr<bsonio::TDBGraph> &value) -> void;
    auto setDataNames(const vector<string> &value) -> void;
    auto setDataHeaders(const vector<string> &value) -> void;
    auto setDataFieldPaths(const vector<string> &value) -> void;
    auto setSubstSymbol_DefinesLevel(const std::map<std::string, std::string> &value) -> void;

    /**
     * @brief queryRecord returns a record queried by id
     * @param idRecord record id in the database
     * @param queryFields schema field paths which are copied and returned in the query result
     * @return string representing the result in JSON format containing the queryFields as keys
     */
    auto queryRecord(string idRecord, vector<string> queryFields) -> string;

    /// Test record existence
    auto recordExists(const string& id ) -> bool;

    /// Add new Vertex record to database
    /// \return oid of new record
    auto addNewRecord( const bsonio::FieldSetMap& fldvalues, bool testValues ) -> string;

    /// Extract values from record into database
    auto loadRecord( const string& id, const vector<string>& queryFields ) -> bsonio::FieldSetMap;

    /// Build table of fields values by ids list
    auto loadRecords( const vector<string>& ids ) -> bsonio::ValuesTable;

    /// Build ids list connected to idInVertex by edge
    auto getInVertexIds(const string& edgeLabel, const string& idVertex) -> vector<string>;

    /// Build ids list connected to idInVertex by edge, record edgesIds
    auto getInVertexIds(const string& edgeLabel, const string& idVertex,  vector<string> &edgesIds) -> vector<string>;

    /// Build ids list connected to idInVertex by edge
    auto getOutVertexIds(const string& edgeLabel, const string& idVertex) -> vector<string>;

    /// Build ids list connected to idInVertex by edge, record edgesIds
    auto getOutVertexIds(const string& edgeLabel, const string& idVertex,  vector<string> &edgesIds) -> vector<string>;

protected:

    /// Test all elements from formula exist into list
    static auto testElementsFormula( const string& aformula, const vector<ElementKey>& elements) -> bool;

    auto resetDataPathIndex() -> void;

    auto setDefaultLevelForReactionDefinedSubst(bsonio::ValuesTable valuesTable) -> void;

    auto getDB_fullAccessMode() const -> boost::shared_ptr<bsonio::TDBGraph>;

    auto queryInEdgesDefines_(string idSubst, vector<string> queryFields,  string level) -> vector<string>;

    auto definesReactionSymbol_(string idSubst, string level) -> std::string;

    auto queryInEdgesTakes_(string idReact, vector<string> queryFields) -> vector<string>;

    auto reactantsCoeff_(string idReact) -> std::map<string, double>;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;

};


}

#endif // ABSTRACTDATA_H
