#include "ReactionSetData.h"
#include "bsonio/io_settings.h"

using namespace bsonio;

namespace ThermoFun
{

const string reactQuery = "{\"_label\": \"reactionset\" }";
const vector<string> reactFieldPaths =
    { "properties.symbol", "properties.name", "properties.stype", "properties.level", "_id"};
const vector<string> reactDataNames = {"symbol", "name", "type", "level", "_id"};
const vector<string> reactColumnHeaders = { "symbol", "name", "stype", "level" };

struct ReactionSetData_::Impl
{
    bsonio::ValuesTable valuesTable;

    Impl( )
    {
    }

};

ReactionSetData_::ReactionSetData_(const bsonio::TDataBase* dbconnect)
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

set<ThermoFun::ElementKey> ReactionSetData_::getElementsList( const string& idrcset )
{
  set<ElementKey> elements; bson obj;
  obj = getJsonBsonRecordVertex(idrcset+":").second;
  ElementsFromBsonArray("properties.elements", obj.data, elements);
//  bson_destroy(&obj);

  // if user fogot tnsert elements property
  if( elements.empty() )
  {
      vector<string> formulalst = getSubstanceFormulas( idrcset );
      elements = ThermoFun::ChemicalFormula::extractElements(formulalst );
  }
  return elements;
}

bsonio::ValuesTable ReactionSetData_::loadRecordsValues( const string& aquery,
                int sourcetdb, const vector<ElementKey>& elements )
{
    // get records by query
    string query = aquery;
    if( query.empty() )
       query = getQuery();
    if( !elements.empty() )
      addFieldsToQuery( query, { make_pair( string("properties.sourcetdb"), to_string(sourcetdb)) } );
    ValuesTable reactQueryMatr = getDB()->loadRecords( query, getDataFieldPaths() );

    // get record by elements list
    ValuesTable reactMatr;
    if( elements.empty() )
    {
       reactMatr = reactQueryMatr;
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

bsonio::ValuesTable ReactionSetData_::loadRecordsValues( const string& idrcset )
{
    vector<string> ids;
    ids.push_back(idrcset);
    return getDB()->loadRecords(ids, getDataFieldPaths());
}

vector<string> ReactionSetData_::getSubstanceFormulas( const string& idrcset )
{
    vector<string> formulas;
    string formSub; bson obj;

    // Select substance ids connected to reactionSet
    auto subIds = getInVertexIds( "product", idrcset );
    auto subIds2 = getInVertexIds( "master", idrcset );
    subIds.insert(  subIds.end(), subIds2.begin(), subIds2.end() );

    // for all substances
    for( auto rec: subIds)
    {
//        getDB()->GetRecord((rec+":").c_str());
//        getDB()->getValue("properties.formula", formSub);
        obj = getJsonBsonRecordVertex(rec+":").second;
        bsonio::bson_to_key( obj.data, "properties.formula", formSub);
        formulas.push_back(formSub);
    }
    return formulas;
}

bool ReactionSetData_::testElements( const string& idrcset,
                       const vector<ElementKey>& elements )
{
   set<ElementKey> reactelements = getElementsList( idrcset );

   /* if user fogot tnsert elements property
   if( reactelements.empty() )
   {
       vector<string> formulalst = getSubstanceFormulas( idrcset );
       reactelements = elements::ChemicalFormula::extractElements(formulalst );
   }*/
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
        graphdb->GetRecord(aKey.c_str());
        graphdb->getValue("_id",_id);

        vector<string> formulalst = getSubstanceFormulas( _id );
        set<ThermoFun::ElementKey> elements = ThermoFun::ChemicalFormula::extractElements(formulalst );

        string elementsJsonArray = ThermoFun::ElementsToJson( elements );
        graphdb->setValue("properties.elements",elementsJsonArray);
        graphdb->SaveCurrent( true, true  );
    }
    catch(bsonio_exeption& e)
    {
        cout << "ResetElementsintoReactionRecord " << e.title() << e.what() << endl;
    }
    catch(std::exception& e)
    {
        cout << "std::exception" << e.what() << endl;
    }
}

}
