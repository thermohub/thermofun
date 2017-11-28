#include "SubstanceData.h"

using namespace bsonio;

namespace ThermoFun {

const string substQuery = "{\"_label\": \"substance\" }";
const vector<string> substFieldPaths =
   { "properties.symbol","properties.name","properties.formula","_id", "properties.class_", "properties.sourcetdb"};
const vector<string> substColumnHeaders = { "symbol", "name", "formula" };
const vector<string> substDataNames  = { "symbol", "name", "formula", "_id", "class_", "sourcetdb" };

struct SubstanceData_::Impl
{
    bsonio::ValuesTable valuesTable;

    Impl( )
    {
    }

};

SubstanceData_::SubstanceData_()
    : AbstractData( "VertexSubstance", substQuery, substFieldPaths, substColumnHeaders, substDataNames), pimpl(new Impl())
{ }

SubstanceData_::SubstanceData_(const SubstanceData_& other)
: AbstractData(other), pimpl(new Impl(*other.pimpl))
{}

auto SubstanceData_::operator=(SubstanceData_ other) -> SubstanceData_&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

SubstanceData_::~SubstanceData_()
{ }

auto SubstanceData_::queryInEdgesDefines(string idSubst, vector<string> queryFields,  string level) -> vector<string>
{
    return queryInEdgesDefines_(idSubst, queryFields, level);
}

auto SubstanceData_::definesReactionSymbol(string idSubst, string level) -> std::string
{
    return definesReactionSymbol_(idSubst, level);
}

auto SubstanceData_::getSubstanceLevel(string substSymbol) const -> string
{
    return getSubstanceLevel_(substSymbol);
}

auto SubstanceData_::setSubstanceLevel(string substSymbol, string level) -> void
{
    setSubstanceLevel_(substSymbol, level);
}

set<ElementKey> SubstanceData_::getElementsList( const string& idSubstance )
{
  string formula; bson record;
  // get recrod
  record = getJsonBsonRecord(idSubstance+":").second;
  // Extract data from fields
  bsonio::bson_to_key( record.data, getDataName_DataFieldPath()["formula"], formula);

  FormulaToken parser(formula);
  return parser.getElements();
}

ValuesTable SubstanceData_::loadRecordsValues( const string& aquery,
                    int sourcetdb, const vector<ElementKey>& elements )
{
    // get records by query
    string query = aquery;
    if (query.empty())
       query = getQuery();
    if (!elements.empty())
      addFieldsToQuery( query, { make_pair( string(getDataName_DataFieldPath()["sourcetdb"]), to_string(sourcetdb)) } );

    ValuesTable substQueryMatr = getDB()->loadRecords(query, getDataFieldPaths());

    // get record by elements list
    ValuesTable substMatr;
    if (elements.empty())
    {
       substMatr = substQueryMatr;
    }
    else
     {  for (const auto& subitem : substQueryMatr)
        {
          string formula = subitem[getDataName_DataIndex()["formula"]];
          if (testElementsFormula(formula, elements))
               substMatr.push_back(subitem);
         }
    }
    setDefaultLevelForReactionDefinedSubst(substMatr);
    pimpl->valuesTable =          substMatr;
    return                substMatr;
}

ValuesTable SubstanceData_::loadRecordsValues( const string& idReactionSet )
{
    auto subIds = getInVertexIds("product", idReactionSet);
    auto subIds2 = getInVertexIds("master", idReactionSet);
    subIds.insert(subIds.end(), subIds2.begin(), subIds2.end());

    ValuesTable substMatr = getDB()->loadRecords(subIds, getDataFieldPaths());
    setDefaultLevelForReactionDefinedSubst(substMatr);
    pimpl->valuesTable = substMatr;
    return substMatr;
}

auto SubstanceData_::querySolvents(int sourcetdb) -> vector<vector<string>>
{
  string qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.class_\" : 3}]}";
  addFieldsToQuery( qrJson, { make_pair( string("properties.sourcetdb"), to_string(sourcetdb)) } );

  ValuesTable solventMatr = getDB()->loadRecords(qrJson, getDataFieldPaths());
  return solventMatr;
}

auto SubstanceData_::nextValueForDefinesLevel (string idSubst) const -> string
{
    // maybe use query edge defines memoized?
    string queryJson, level = "0"; ValuesTable levelQueryMatr;
    vector<int> levels;
    // check if more edge defines are connected to this substance
    queryJson = "{'_type': 'edge', '_label': 'defines', '_inV': '";
    queryJson += idSubst;
    queryJson += "'}";

    levelQueryMatr = getDB()->loadRecords( queryJson, {"properties.level"} );
    for (uint i = 0; i < levelQueryMatr.size(); i++)
    {
        levels.push_back(std::stoi(levelQueryMatr[i][0]));
    }
    if (levels.size() > 0)
    level = std::to_string((*std::max_element(levels.begin(), levels.end())+1));
    return level;
}

MapSubstSymbol_MapLevel_IdReaction SubstanceData_::recordsMapLevelDefinesReaction( )
{
    MapSubstSymbol_MapLevel_IdReaction recordsLevelReact; bson record;
    for (auto value : pimpl->valuesTable)
    {
        MapLevel_IdReaction levelReact;
        // returns the ids of reactions which are conncted to the substance with id value[3] with edge defines
        vector<string> resultDefinesEdges;
        vector<string> resultDefinesReactions = getOutVertexIds( "defines", value[getDataName_DataIndex()["_id"]], resultDefinesEdges );
        for (uint i = 0; i < resultDefinesReactions.size(); i++)
        {
            string level;
            record = getJsonBsonRecord((resultDefinesEdges[i]+":").c_str()).second;
            // Extract data from fields
            bsonio::bson_to_key( record.data, "properties.level", level);
            levelReact[level] = resultDefinesReactions[i]+":";
        }
//        if (!levelReact.empty())
        recordsLevelReact[value[getDataName_DataIndex()["symbol"]]] = levelReact;
    }
    return recordsLevelReact;
}

MapSubstSymbol_MapLevel_IdReaction SubstanceData_::recordsMapLevelDefinesReaction(vector<string> connectedSubstIds, vector<string> connectedSubstSymbols )
{
    MapSubstSymbol_MapLevel_IdReaction recordsLevelReact; bson record;
    for (uint i = 0; i < connectedSubstIds.size(); i++)
    {
        MapLevel_IdReaction levelReact;
        // returns the ids of reactions which are conncted to the substance with id value[3] with edge defines
        vector<string> resultDefinesEdges;
        vector<string> resultDefinesReactions = getOutVertexIds( "defines", connectedSubstIds[i], resultDefinesEdges );
        for (uint i = 0; i < resultDefinesReactions.size(); i++)
        {
            string level;
            record = getJsonBsonRecord((resultDefinesEdges[i]+":").c_str()).second;
            // Extract data from fields
            bsonio::bson_to_key( record.data, "properties.level", level);
            levelReact[level] = resultDefinesReactions[i]+":";
        }
//        if (!levelReact.empty())
        recordsLevelReact[connectedSubstSymbols[i]] = levelReact;
    }
    return recordsLevelReact;
}

}
