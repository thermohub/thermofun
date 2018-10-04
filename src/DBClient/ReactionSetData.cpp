#include "ReactionSetData.h"
#include "jsonio/io_settings.h"
#include "jsonio/jsondomfree.h"

using namespace jsonio;

namespace ThermoFun
{

//const DBQueryData reactQuery( "{\"_label\": \"reactionset\" }", DBQueryData:: qTemplate );
const DBQueryData reactQuery("FOR u  IN reactionsets ", DBQueryData::qAQL);
const vector<string> reactFieldPaths =
    { "properties.symbol", "properties.name", "properties.stype", "properties.level", "_id"};
const vector<string> reactDataNames = {"symbol", "name", "type", "level", "_id"};
const vector<string> reactColumnHeaders = { "symbol", "name", "stype", "level" };

struct ReactionSetData_::Impl
{
    ValuesTable valuesTable;

    Impl( )
    {
    }

};

ReactionSetData_::ReactionSetData_(const TDataBase* dbconnect)
    : AbstractData(  dbconnect, "VertexReactionSet", reactQuery,
     reactFieldPaths, reactColumnHeaders, reactDataNames), pimpl(new Impl())
{
}

ReactionSetData_::ReactionSetData_(const ReactionSetData_& other)
: AbstractData(other), pimpl(new Impl(*other.pimpl))
{}

auto ReactionSetData_::operator=(ReactionSetData_ other) -> ReactionSetData_ &
{
    pimpl = std::move(other.pimpl);
    return *this;
}

ReactionSetData_::~ReactionSetData_()
{
}

const jsonio::ValuesTable&  ReactionSetData_::getValuesTable()
{
    return pimpl->valuesTable;
}

set<ThermoFun::ElementKey> ReactionSetData_::getElementsList( const string& idrcset )
{
  set<ElementKey> elements;
  string jsonrecord = getJsonRecordVertex(idrcset);
  auto domdata = jsonio::unpackJson( jsonrecord );
  ElementsFromJsonDomArray("properties.elements", domdata.get(), elements);

  // if user fogot tnsert elements property
  if( elements.empty() )
  {
      vector<string> formulalst = getSubstanceFormulas( idrcset );
      elements = ThermoFun::ChemicalFormula::extractElements(formulalst );
  }
  return elements;
}

// return all record, no only fields if not default
ValuesTable ReactionSetData_::loadRecordsValues( const DBQueryData& aquery,
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
      addFieldsToQueryAQL( query, { make_pair( string("properties.sourcetdb"), to_string(sourcetdb)) } );
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
          if( testElements( idreac, elements)  )
               reactMatr.push_back(subitem);
        }
    }
    return reactMatr;
}

ValuesTable ReactionSetData_::loadRecordsValues( const string& idrcset )
{
    vector<string> ids;
    ids.push_back(idrcset);
    return getDB()->downloadDocuments(ids, getDataFieldPaths());
}

vector<string> ReactionSetData_::getSubstanceIds( const string& idrcset )
{
    // Select substance ids connected to reactionSet
    auto subIds = getInVertexIds( "product, master", idrcset );
    //auto subIds2 = getInVertexIds( "master", idrcset );
    //subIds.insert(  subIds.end(), subIds2.begin(), subIds2.end() );
    return subIds;
}


vector<string> ReactionSetData_::getSubstanceFormulas( const string& idrcset )
{
    string qrJson = "FOR v,e  IN 1..1 INBOUND '";
           qrJson += idrcset + "' \n product, master\n";
           qrJson += "RETURN DISTINCT v.properties.formula";

    vector<string> formulas = getDB()->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ) );
    return formulas;
}

bool ReactionSetData_::testElements( const string& idrcset,
                       const vector<ElementKey>& elements )
{
   set<ElementKey> reactelements = getElementsList( idrcset );

   if(reactelements.empty())
     return false;

   for( auto formelm: reactelements )
    {
      auto itr = elements.begin();
      while( itr != elements.end() )
      {
        if( formelm == *itr )
         break;
       itr++;
      }
      if( itr == elements.end() )
          return false;
    }
    return true;
}

void ReactionSetData_::resetRecordElements( const string& aKey )
{
    string _id;
    try{
        auto graphdb = getDB();
        graphdb->Read( aKey );
        graphdb->getValue("_id",_id);

        vector<string> formulalst = getSubstanceFormulas( _id );
        set<ThermoFun::ElementKey> elements = ThermoFun::ChemicalFormula::extractElements(formulalst );

        string elementsJsonArray = ThermoFun::ElementsToJson( elements );
        graphdb->setValue("properties.elements",elementsJsonArray);
        graphdb->SaveCurrent( true, true  );
    }
    catch(jsonio_exception& e)
    {
        cout << "ResetElementsintoReactionRecord " << e.title() << e.what() << endl;
    }
    catch(std::exception& e)
    {
        cout << "std::exception" << e.what() << endl;
    }
}

bool ReactionSetData_::getSpeciesMap( const string& RcSid, std::map<string, int>& specmap )
{
    // extract data from reaction record
    getDB()->Read( RcSid );
    bool iret = getDB()->getDom()->findObject(  "properties.species_map", specmap );
    //bool iret = bson_read_map_path( reobj.data, "properties.species_map", specmap );
    return iret;
}

vector<string> ReactionSetData_::selectGivenSubstances( const vector<int>& sourcetdbs,
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
    domdata->appendArray( "sourcetdbs", sourcetdbs );
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

vector<string> ReactionSetData_::selectGiven(const vector<int>& sourcetdbs,
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
    domdata->appendArray( "sourcetdbs", sourcetdbs );
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

vector<string> ReactionSetData_::selectGiven( const vector<string>& idThermoDataSets, bool unique )
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

vector<string> ReactionSetData_::selectGiven( const string& idThermoDataSet, const vector<string>& reactionSymbols )
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
