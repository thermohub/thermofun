#ifndef ABSTRACTDATA_H
#define ABSTRACTDATA_H

// C++ includes
#include <memory>

// ThermoFun includes
#include "formuladata.h"
#include "jsonio/dbconnect.h"
#include "jsonio/dbedgedoc.h"

namespace ThermoFun {

/// Temporaly from jsonio
void addFieldsToQuery( string& query, const jsonio::FieldSetMap& fldvalues );

class AbstractData
{
public:

    AbstractData( const jsonio::TDataBase* dbconnect,
                  const string &name, const string &query, const vector<string> &paths,
                  const vector<string> &headers, const vector<string> &names );

    /// Construct a copy of an ThermoDataAbstract instance
    AbstractData(const AbstractData& other);

    /// Destroy this instance
    virtual ~AbstractData();

    // load values function
    /// Extract data connected to ReactionSet
    virtual jsonio::ValuesTable  loadRecordsValues( const string& idReactionSet ) = 0;
    /// Extract data by condition
    virtual jsonio::ValuesTable  loadRecordsValues( const string& query, int sourcetdb,
                                                    const vector<ElementKey>& elements = {} ) = 0;
    /// Get Elements list from record
    virtual set<ElementKey> getElementsList( const string& idrec ) = 0;

    auto updateDBClient( const jsonio::TDataBase* newdbconnect ) -> void;

    auto getDB() const -> std::shared_ptr<jsonio::TDBVertexDocument>;
    auto getName() const -> string;
    auto getQuery() const -> string;
    auto getDataNames() const -> vector<string>;
    auto getDataHeaders() const -> vector<string>;
    auto getDataFieldPaths() const -> vector<string>;
    auto getDataName_DataIndex() const -> std::map<std::string, int>;
    auto getDataName_DataFieldPath() const -> std::map<std::string, std::string>;
    auto getSubstSymbol_DefinesLevel() const -> std::map<std::string, std::string>;

///    auto setDB(const std::shared_ptr<bsonio::TDBVertexDocument> &value) -> void;
///    auto setDataNames(const vector<string> &value) -> void;
    auto setDataHeaders(const vector<string> &value) -> void;
    auto setDataFieldPaths(const vector<string> &value) -> void;
    auto setSubstSymbol_DefinesLevel(const std::map<std::string, std::string> &value) -> void;
    auto setDataNamesHeadersFieldpaths(const vector<string> &names, const vector<string> &headers, const vector<string> &fieldpaths) -> void;

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
    auto addNewRecord( const jsonio::FieldSetMap& fldvalues, bool testValues ) -> string;

    /// Extract values from record into database
    auto loadRecord( const string id, const vector<string> queryFields ) -> jsonio::FieldSetMap;

    /// Build table of fields values by ids list
    auto loadRecords( const vector<string> ids ) -> jsonio::ValuesTable;

    /// Build ids list connected to idVertex by incoming edge
    auto getInVertexIds(const string& edgeLabel, const string& idVertex) -> vector<string>;

    /// Build ids list connected to idVertex by incoming edge, save edgesIds
    auto getInVertexIds(const string& edgeLabel, const string& idVertex,  vector<string> &edgesIds) -> vector<string>;

    /// Build ids list connected to idVertex by outgoing edge
    auto getOutVertexIds(const string& edgeLabel, const string& idVertex) -> vector<string>;

    /// Build ids list connected to idVertex by outgoing edge, save edgesIds
    auto getOutVertexIds(const string& edgeLabel, const string& idVertex,  vector<string> &edgesIds) -> vector<string>;

    /// Returns the record with idRecord as a pair of Json and Bson formats
    auto getJsonRecordVertex(string idRecord) -> std::string;
    /// Returns the record with idRecord as a pair of Json and Bson formats
    auto getJsonRecordEdge(string idRecord) -> std::string;

protected:

    // Returns a record from a database in JSON format using the record id
    auto getJsonRecord(string idRecord) -> string;
    // Test all elements from formula exist into list
    static auto testElementsFormula( const string& aformula, const vector<ElementKey>& elements) -> bool;
    // Resets the data index and data names maps which connect the names to headers to paths
    auto resetDataPathIndex() -> void;
    // sets the level = 0 for substances in ValuesTable
    auto setDefaultLevelForReactionDefinedSubst(jsonio::ValuesTable valuesTable) -> void;
    // returns the full access mode database connection (allows queryies on all types of records)
    auto getDB_edgeAccessMode() const -> std::shared_ptr<jsonio::TDBEdgeDocument>;
    // query the ids of incoming edges of type defines
    auto queryInEdgesDefines_(string idSubst, vector<string> queryFields,  string level) -> vector<string>;
    // returns the reaction symbol which defines a substance
    auto definesReactionSymbol_(string idSubst, string level) -> std::string;
    //  returns data of the incoming edges of type takes
    auto queryInEdgesTakes_(string idReact, vector<string> queryFields) -> vector<string>;
    // returns the map of reactants symbols and coefficients
    auto reactantsCoeff_(string idReact) -> std::map<string, double>;
    // returns the level which is set for the substance
    auto getSubstanceLevel_(string substSymbol) const -> string;
    // sets the level
    auto setSubstanceLevel_(string substSymbol, string level) -> void;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;

};


}

#endif // ABSTRACTDATA_H
