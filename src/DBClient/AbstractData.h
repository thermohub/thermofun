#ifndef ABSTRACTDATA_H
#define ABSTRACTDATA_H

// C++ includes
#include <memory>

// ThermoFun includes
#include "formuladata.h"
#include "jsonio/dbconnect.h"
#include "jsonio/dbedgedoc.h"

namespace ThermoFun {

extern const std::string ThermoDataSetQueryEdges;

class AbstractData
{
public:

    AbstractData( const jsonio::TDataBase* dbconnect,
                  const std::string &name, const jsonio::DBQueryData& query, const std::vector<std::string> &paths,
                  const std::vector<std::string> &headers, const std::vector<std::string> &names );

    /// Construct a copy of an ThermoDataAbstract instance
    AbstractData(const AbstractData& other);

    /// Destroy this instance
    virtual ~AbstractData();

    // load values function
    /// Extract data connected to ReactionSet
    virtual jsonio::ValuesTable  loadRecordsValues( const std::string& idReactionSet ) = 0;
    /// Extract data by condition
    virtual jsonio::ValuesTable  loadRecordsValues( const jsonio::DBQueryData& query, int sourcetdb,
                                                    const std::vector<ElementKey>& elements = {} ) = 0;
    /// Extract data connected to ThermoDataSet
    virtual  std::vector<std::string> selectGiven( const std::vector<std::string>& idThermoDataSets, bool unique = true ) = 0 ;

    /// Get Elements list from record
    virtual std::set<ElementKey> getElementsList( const std::string& idrec ) = 0;

    /// Link to table of fields values loaded before
    virtual const jsonio::ValuesTable& getValuesTable() = 0;

    auto updateDBClient( const jsonio::TDataBase* newdbconnect ) -> void;

    auto getDB() const -> std::shared_ptr<jsonio::TDBVertexDocument>;
    auto getName() const -> std::string;
    auto getQuery() const -> jsonio::DBQueryData;
    auto getDataNames() const -> std::vector<std::string>;
    auto getDataHeaders() const -> std::vector<std::string>;
    auto getDataFieldPaths() const -> std::vector<std::string>;
    auto getDataName_DataIndex() const -> std::map<std::string, uint>;
    auto getDataName_DataFieldPath() const -> std::map<std::string, std::string>;
    auto getSubstSymbol_DefinesLevel() const -> std::map<std::string, std::string>;

    auto makeQueryFields() const -> jsonio::QueryFields;

///    auto setDB(const std::shared_ptr<bsonio::TDBVertexDocument> &value) -> void;
///    auto setDataNames(const vector<string> &value) -> void;
    auto setDataHeaders(const std::vector<std::string> &value) -> void;
    auto setDataFieldPaths(const std::vector<std::string> &value) -> void;
    auto setSubstSymbol_DefinesLevel(const std::map<std::string, std::string> &value) -> void;
    auto setDataNamesHeadersFieldpaths(const std::vector<std::string> &names, const std::vector<std::string> &headers, const std::vector<std::string> &fieldpaths) -> void;
    /// Execute query before load impex to compare data
    auto loadQueryData() -> void;

    /**
     * @brief queryRecord returns a record queried by id
     * @param idRecord record id in the database
     * @param queryFields schema field paths which are copied and returned in the query result
     * @return string representing the result in JSON format containing the queryFields as keys
     */
    auto queryRecord(std::string idRecord, std::vector<std::string> queryFields) -> std::string;

    /// Test record existence
    auto recordExists(const std::string& id ) -> bool;

    /// Add new Vertex record to database
    /// \return oid of new record
    auto CreateRecord( const jsonio::FieldSetMap& fldvalues, bool testValues ) -> std::string;

    /// Extract values from record into database
    auto loadRecord( const std::string id, const std::vector<std::string> queryFields ) -> jsonio::FieldSetMap;

    /// Build table of fields values by ids list
    auto loadRecords( const std::vector<std::string>& ids ) -> jsonio::ValuesTable;


    /// Build ids list connected to idVertex by incoming edge,
    /// edgeCollections ( "coll1, coll2") list of collections to search into
    auto getInVertexIds(const std::string& edgeCollections, const std::string& idVertex) -> std::vector<std::string>;

    /// Build ids list connected to idVertex by incoming edge, save edgesIds
    /// edgeCollections ( "coll1, coll2") list of collections to search into
    auto getInVertexIds(const std::string& edgeCollections, const std::string& idVertex,  std::vector<std::string> &edgesIds) -> std::vector<std::string>;

    /// Build ids list connected to idVertex by outgoing edge,
    /// edgeCollections ( "coll1, coll2") list of collections to search into
    auto getOutVertexIds(const std::string& edgeCollections, const std::string& idVertex) -> std::vector<std::string>;

    /// Build ids list connected to idVertex by outgoing edge, save edgesIds
    /// edgeCollections ( "coll1, coll2") list of collections to search into
    auto getOutVertexIds(const std::string& edgeCollections, const std::string& idVertex,  std::vector<std::string> &edgesIds) -> std::vector<std::string>;

    /// Returns the record with idRecord as a pair of Json and Bson formats
    auto getJsonRecordVertex(std::string idRecord) -> std::string;
    /// Returns the record with idRecord as a pair of Json and Bson formats
    auto getJsonRecordEdge(std::string idRecord) -> std::string;

    auto selectElementsGiven( const std::vector<int>& sourcetdbs, bool unique = true ) -> std::vector<ElementKey>;
    auto selectElementsFromSubstancesGiven( const std::vector<int>& sourcetdbs ) -> std::set<ElementKey>;

protected:

    // Returns a record from a database in JSON format using the record id
    auto getJsonRecord(std::string idRecord) -> std::string;
    // Test all elements from formula exist into list
    static auto testElementsFormula( const std::string& aformula, const std::vector<ElementKey>& elements) -> bool;
    // Resets the data index and data names maps which connect the names to headers to paths
    auto resetDataPathIndex() -> void;
    // sets the level = 0 for substances in ValuesTable
    auto setDefaultLevelForReactionDefinedSubst(jsonio::ValuesTable avaluesTable) -> void;
    // returns the full access mode database connection (allows queryies on all types of records)
    auto getDB_edgeAccessMode() const -> std::shared_ptr<jsonio::TDBEdgeDocument>;
    // query the ids of incoming edges of type defines
    auto queryInEdgesDefines_(std::string idSubst, std::string level) -> std::vector<std::string>;
    // returns the reaction symbol which defines a substance
    auto definesReactionSymbol_(std::string idSubst, std::string level) -> std::string;
    //  returns data of the incoming edges of type takes
    auto queryInEdgesTakes_(std::string idReact ) -> std::vector<std::string>;
    // returns the map of reactants symbols and coefficients
    auto reactantsCoeff_(std::string idReact) -> std::map<std::string, double>;
    // returns the level which is set for the substance
    auto getSubstanceLevel_(std::string substSymbol) const -> std::string;
    // sets the level
    auto setSubstanceLevel_(std::string substSymbol, std::string level) -> void;

    void deleteNotUnique(jsonio::ValuesTable& dataMatr, uint fldtestNdx );

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;

};


}

#endif // ABSTRACTDATA_H
