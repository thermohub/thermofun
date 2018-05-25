#include <algorithm>
#include "SubstanceData.h"
#include "jsonio/jsondomfree.h"

using namespace jsonio;

namespace ThermoFun {

//const DBQueryData substQuery("{\"_label\": \"substance\" }", DBQueryData::qTemplate);
const DBQueryData substQuery("FOR u  IN substances ", DBQueryData::qAQL);
const vector<string> substFieldPaths =
   { "properties.symbol","properties.name","properties.formula","_id", "properties.class_", "properties.sourcetdb"};
const vector<string> datsetColumnHeaders = { "symbol", "name", "formula" };
const vector<string> substDataNames  = { "symbol", "name", "formula", "_id", "class_", "sourcetdb" };

struct SubstanceData_::Impl
{
    ValuesTable valuesTable;

    Impl( )
    {
    }

};


SubstanceData_::SubstanceData_( const TDataBase* adbconnect )
    : AbstractData( adbconnect, "VertexSubstance", substQuery,
                    substFieldPaths, datsetColumnHeaders, substDataNames), pimpl(new Impl())
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
  string formula;
  // get record
  string jsonrecord = getJsonRecordVertex(idSubstance+":");
  auto domdata = jsonio::unpackJson( jsonrecord );

  // Extract data from fields
  domdata->findKey(getDataName_DataFieldPath()["formula"], formula);
  //bsonio::bson_to_key( record.data, getDataName_DataFieldPath()["formula"], formula);

  FormulaToken parser(formula);
  return parser.getElements();
}


void SubstanceData_::updateTableByElementsList( ValuesTable& substQueryMatr, const vector<ElementKey>& elements )
{
    if( !elements.empty() )
    {
        ValuesTable substMatr;
        for (const auto& subitem : substQueryMatr)
        {
         string formula = subitem[getDataName_DataIndex()["formula"]];
         if (testElementsFormula(formula, elements))
              substMatr.push_back(subitem);
        }
        substQueryMatr = move(substMatr);
    }
 }

// return all record, no only fields if not default
ValuesTable SubstanceData_::loadRecordsValues( const DBQueryData& aquery,
                    int sourcetdb, const vector<ElementKey>& elements )
{
    auto fields = getDataFieldPaths();

    // get records by query
    auto query = aquery;
    if (query.empty())
    {
        query = getQuery();
        // only here we have subset fields to extract
        query.setQueryFields(makeQueryFields());
        fields = getDataNames();
    }
    //if (!elements.empty())
      addFieldsToQueryAQL( query, { make_pair( string(getDataName_DataFieldPath()["sourcetdb"]), to_string(sourcetdb)) } );

    ValuesTable substQueryMatr = getDB()->loadRecords(query, fields);

    // get record by elements list
    updateTableByElementsList( substQueryMatr, elements );
    setDefaultLevelForReactionDefinedSubst(substQueryMatr);
    pimpl->valuesTable = substQueryMatr;
    return               move(substQueryMatr);
}

ValuesTable SubstanceData_::loadRecordsValues( const string& idReactionSet )
{
    string qrAQL = "FOR u,e  IN 1..1 INBOUND '";
           qrAQL += idReactionSet + "' \n product, master ";
    //       qrAQL += "RETURN u";
    DBQueryData query( qrAQL, DBQueryData::qAQL );
    query.setQueryFields(makeQueryFields());
    ValuesTable substMatr =  getDB()->loadRecords( query, getDataNames());
    setDefaultLevelForReactionDefinedSubst(substMatr);
    pimpl->valuesTable = substMatr;
    return substMatr;
}

auto SubstanceData_::querySolvents(int sourcetdb) -> vector<vector<string>>
{
  string aqlStr = "FOR u  IN substances\n  FILTER u.properties.class_ == 3 && u.properties.sourcetdb == ";
        aqlStr +=  to_string(sourcetdb) + " ";
  auto qrJson = DBQueryData( aqlStr, DBQueryData::qAQL );
  qrJson.setQueryFields(makeQueryFields());

 ValuesTable solventMatr = getDB()->loadRecords(qrJson, getDataNames() );
 return solventMatr;
}

auto SubstanceData_::nextValueForDefinesLevel (string idSubst) const -> string
{
    // maybe use query edge defines memoized?
    string level = "0"; ValuesTable levelQueryMatr;
    vector<int> levels;

    auto queryJson = getDB_edgeAccessMode()->inEdgesQuery( "defines", idSubst);
    queryJson.setQueryFields( {{ "level", "properties.level" }} );
    levelQueryMatr = getDB_edgeAccessMode()->loadRecords( queryJson, {"level"} );
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
    string level, idreact, idsub;
    MapSubstSymbol_MapLevel_IdReaction recordsLevelReact;
    for (auto value : pimpl->valuesTable)
    {
        MapLevel_IdReaction levelReact;
        idsub = value[getDataName_DataIndex()["_id"]];
        string qrJson = "FOR v,e  IN 1..1 INBOUND '";
               qrJson += idsub + "' \n defines\n";
               qrJson += "RETURN { 'level': e.properties.level, 'reaction': v._id }";

        vector<string> resultsQuery;
        getDB()->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ),  {}, resultsQuery);

        for( auto result: resultsQuery )
        {
            auto domdata = jsonio::unpackJson( result );
            domdata->findKey("level", level);
            domdata->findKey("reaction", idreact);
            levelReact[level] = idreact+":";
        }
//        if (!levelReact.empty())
        recordsLevelReact[value[getDataName_DataIndex()["symbol"]]] = levelReact;
    }
    return recordsLevelReact;
}


MapSubstSymbol_MapLevel_IdReaction SubstanceData_::recordsMapLevelDefinesReaction(vector<string> connectedSubstIds, vector<string> connectedSubstSymbols )
{
    string level, idreact, idsub;
    MapSubstSymbol_MapLevel_IdReaction recordsLevelReact;

    for (uint i = 0; i < connectedSubstIds.size(); i++)
    {
        MapLevel_IdReaction levelReact;
        idsub = connectedSubstIds[i];
        string qrJson = "FOR v,e  IN 1..1 INBOUND '";
               qrJson += idsub + "' \n defines\n";
               qrJson += "RETURN { 'level': e.properties.level, 'reaction': v._id }";

        vector<string> resultsQuery;
        getDB()->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ),  {}, resultsQuery);

        for( auto result: resultsQuery )
        {
            auto domdata = jsonio::unpackJson( result );
            domdata->findKey("level", level);
            domdata->findKey("reaction", idreact);
            levelReact[level] = idreact+":";
        }
//        if (!levelReact.empty())
        recordsLevelReact[connectedSubstSymbols[i]] = levelReact;
    }
    return recordsLevelReact;
}

vector<string> SubstanceData_::selectGiven( const vector<int>& sourcetdbs,
                   const vector<ElementKey>& elements, bool unique )
{
    // define query string
    string AQLreq = "FOR u IN substances \n"
                    "  FILTER u.properties.sourcetdb IN @sourcetdbs\n"
                    "  SORT u.properties.symbol ";

    // generate bind values
    shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
    domdata->appendArray( "sourcetdbs", sourcetdbs );
    // make query
    DBQueryData query( AQLreq, DBQueryData::qAQL );
    query.setBindVars( domdata.get() );
    query.setQueryFields( makeQueryFields() );

    ValuesTable substQueryMatr = getDB()->loadRecords(query, getDataNames());

    // delete not unique
    if( unique )
    {
        ValuesTable substMatr;
        for (const auto& subitem : substQueryMatr)
        {
           auto symbol = subitem[getDataName_DataIndex()["symbol"]];
           if ( substMatr.empty() ||
                substMatr.back()[getDataName_DataIndex()["symbol"]] != symbol )
                       substMatr.push_back(subitem);
        }
       substQueryMatr = move(substMatr);
    }

    updateTableByElementsList( substQueryMatr, elements );

    vector<string> substanceSymbols;
    for (const auto& subitem : substQueryMatr)
      substanceSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

    setDefaultLevelForReactionDefinedSubst(substQueryMatr);
    pimpl->valuesTable =          move(substQueryMatr);
    return                substanceSymbols;
}

}
