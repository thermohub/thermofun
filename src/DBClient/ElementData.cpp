#include "ElementData.h"
#include "jsonio/io_settings.h"
#include "jsonio/jsondomfree.h"
#include "sourcetdb.h"

using namespace std;
using namespace jsonio;

namespace ThermoFun
{

//const DBQueryData reactQuery( "{\"_label\": \"reactionset\" }", DBQueryData:: qTemplate );
const DBQueryData elementQuery("FOR u  IN reactionsets ", DBQueryData::qAQL);
const vector<string> elementFieldPaths =
    { "properties.symbol", "properties.name", "properties.stype", "properties.level", "_id"};
const vector<string> elementDataNames = {"symbol", "name", "type", "level", "_id"};
const vector<string> elementColumnHeaders = { "symbol", "name", "stype", "level" };

struct ElementData_::Impl
{
    ValuesTable valuesTable;

    Impl( )
    {
    }

};

ElementData_::ElementData_(const TDataBase* dbconnect)
    : AbstractData(  dbconnect, "VertexElement", elementQuery,
     elementFieldPaths, elementColumnHeaders, elementDataNames), pimpl(new Impl())
{
}

ElementData_::ElementData_(const ElementData_& other)
: AbstractData(other), pimpl(new Impl(*other.pimpl))
{}

auto ElementData_::operator=(ElementData_ other) -> ElementData_ &
{
    pimpl = std::move(other.pimpl);
    return *this;
}

ElementData_::~ElementData_()
{
}

const jsonio::ValuesTable&  ElementData_::getValuesTable()
{
    return pimpl->valuesTable;
}

set<ThermoFun::ElementKey> ElementData_::getElementsList( const string& idrcset )
{
  set<ElementKey> elements;
  string jsonrecord = getJsonRecordVertex(idrcset);
  auto domdata = jsonio::unpackJson( jsonrecord );
  ElementsKeysFromJsonDomArray("properties.elements", domdata.get(), elements);

  // if user fogot tnsert elements property
  if( elements.empty() )
  {
      //vector<string> formulalst = getSubstanceFormulas( idrcset );
      //elements = ThermoFun::ChemicalFormula::extractElements(formulalst );
  }
  return elements;
}

// return all record, no only fields if not default
ValuesTable ElementData_::loadRecordsValues( const DBQueryData& aquery,
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
    if( !elements.empty() )
      addFieldsToQueryAQL( query, { make_pair( string("properties.sourcetdb"), sourceTDB_from_index(sourcetdb)) } );
    ValuesTable reactQueryMatr = getDB()->downloadDocuments( query, fields );

    // get record by elements list
    ValuesTable reactMatr;
    if( elements.empty() )
    {
       reactMatr = move(reactQueryMatr);
    }
    else
     {
       for( const auto& subitem: reactQueryMatr )
        {
          string idreac = subitem[getDataName_DataIndex()["_id"]];
          //if( testElements( idreac, elements)  )
          //     reactMatr.push_back(subitem);
        }
    }
    return reactMatr;
}

ValuesTable ElementData_::loadRecordsValues( const string& idrcset )
{
    vector<string> ids;
    ids.push_back(idrcset);
    return getDB()->downloadDocuments(ids, getDataFieldPaths());
}



vector<string> ElementData_::selectGivenSubstances( const vector<int>& sourcetdbs,
                   const vector<string>& substanceSymbols, bool unique )
{
    // define query string
    string AQLreq = "FOR u IN reactionsets \n"
                    "  FILTER u.properties.sourcetdb IN @sourcetdbs\n"
                    "  LET takesreac = ( FOR v,e IN 1..1 INBOUND u._id prodreac  RETURN v.properties.symbol ) \n"
                    "  FILTER takesreac ALL IN @substanceSymbols \n"
                    "  SORT u.properties.symbol ";
           AQLreq += DBQueryData::generateReturn( false, makeQueryFields());

    // generate bind values
    shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
    auto arr = domdata->appendArray( "sourcetdbs");
    sourceTDB_from_indexes( sourcetdbs, arr );
    domdata->appendArray( "substanceSymbols", substanceSymbols);
    // make query
    DBQueryData query( AQLreq, DBQueryData::qAQL );
    query.setBindVars( domdata.get() );
    query.setQueryFields( makeQueryFields() );

    ValuesTable reactSetQueryMatr = getDB()->downloadDocuments(query, getDataNames());

    // delete not unique
    if( unique )
        deleteNotUnique( reactSetQueryMatr, getDataName_DataIndex()["symbol"] );

    vector<string> reactSetSymbols;
    for (const auto& subitem : reactSetQueryMatr)
      reactSetSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

    setDefaultLevelForReactionDefinedSubst(reactSetQueryMatr);
    pimpl->valuesTable =          move(reactSetQueryMatr);
    return                reactSetSymbols;
}

vector<string> ElementData_::selectGiven(const vector<int>& sourcetdbs,
                   const vector<string>& reactionSymbols, bool unique )
{
    // define query string
    string AQLreq = "FOR u IN reactionsets \n"
                    "  FILTER u.properties.sourcetdb IN @sourcetdbs\n"
                    "  LET prodreac = ( FOR v,e IN 1..1 INBOUND u._id prodreac  RETURN v.properties.symbol ) \n"
                    "  FILTER prodreac ALL IN @reactionSymbols \n"
                    "  SORT u.properties.symbol ";
           AQLreq += DBQueryData::generateReturn( false, makeQueryFields());

    // generate bind values
    shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
    auto arr = domdata->appendArray( "sourcetdbs");
    sourceTDB_from_indexes( sourcetdbs, arr );
    domdata->appendArray( "reactionSymbols", reactionSymbols);
    // make query
    DBQueryData query( AQLreq, DBQueryData::qAQL );
    query.setBindVars( domdata.get() );
    query.setQueryFields( makeQueryFields() );

    ValuesTable reactSetQueryMatr = getDB()->downloadDocuments(query, getDataNames());

    // delete not unique
    if( unique )
        deleteNotUnique( reactSetQueryMatr, getDataName_DataIndex()["symbol"] );

    vector<string> reactSetSymbols;
    for (const auto& subitem : reactSetQueryMatr)
      reactSetSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

    setDefaultLevelForReactionDefinedSubst(reactSetQueryMatr);
    pimpl->valuesTable =          move(reactSetQueryMatr);
    return                reactSetSymbols;
}

vector<string> ElementData_::selectGiven( const vector<string>& idThermoDataSets, bool unique )
{
    string qrAQL = "FOR vertex IN " + vectorToJson( idThermoDataSets);
           qrAQL += "\n  FOR v,e  IN 1..5 INBOUND vertex \n";
           qrAQL +=  ThermoDataSetQueryEdges;
           qrAQL +=  "\n  FILTER v._label == 'reactionset' ";
           qrAQL +=  "\n  SORT v.properties.symbol ";
           qrAQL +=  DBQueryData::generateReturn( true, makeQueryFields(), "v");
    DBQueryData query( qrAQL, DBQueryData::qAQL );
    ValuesTable resMatr =  getDB()->downloadDocuments( query, getDataNames());

    if( unique )
        deleteNotUnique( resMatr, getDataName_DataIndex()["symbol"] );

    vector<string> reacSymbols;
    for (const auto& subitem : resMatr)
      reacSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

    setDefaultLevelForReactionDefinedSubst(resMatr);
    pimpl->valuesTable = move(resMatr);
    return reacSymbols;
}

vector<string> ElementData_::selectGiven( const string& idThermoDataSet, const vector<string>& reactionSymbols )
{
    string qrAQL =  "FOR v  IN 1..5 INBOUND '" + idThermoDataSet + "'\n";
           qrAQL +=  ThermoDataSetQueryEdges;
           qrAQL +=  "\n  FILTER v._label == 'reactionset' ";
           qrAQL +=  "\n  LET prodreac = ( FOR v1 IN 1..1 INBOUND v._id prodreac  RETURN v1.properties.symbol )";
           qrAQL +=  "\n  FILTER prodreac ALL IN @reactionSymbols";
           qrAQL +=  "\n  SORT v.properties.symbol ";
           qrAQL +=  DBQueryData::generateReturn( true, makeQueryFields(), "v");

     // generate bind values
     shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
     domdata->appendArray( "reactionSymbols", reactionSymbols);
     // make query
     DBQueryData query( qrAQL, DBQueryData::qAQL );
     query.setBindVars( domdata.get() );
     ValuesTable resMatr =  getDB()->downloadDocuments( query, getDataNames());

     vector<string> reacSymbols;
     for (const auto& subitem : resMatr)
       reacSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

     setDefaultLevelForReactionDefinedSubst(resMatr);
     pimpl->valuesTable = move(resMatr);
     return reacSymbols;
}


}
