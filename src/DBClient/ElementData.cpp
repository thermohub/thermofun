#include "ElementData.h"
#include "jsonio/io_settings.h"
#include "jsonio/jsondomfree.h"
#include "sourcetdb.h"

using namespace std;
using namespace jsonio;

namespace ThermoFun
{

//const DBQueryData reactQuery( "{\"_label\": \"element\" }", DBQueryData:: qTemplate );
const DBQueryData elementQuery("FOR u  IN elements ", DBQueryData::qAQL);
const vector<string> elementFieldPaths = { "properties.symbol",  "properties.class_", "properties.isotope_mass",
                                           "properties.name", "properties.number", "_id" };;
const vector<string> elementDataNames = {"symbol",  "class_", "isotope_mass", "name", "number", "_id" };
const vector<string> elementColumnHeaders = { "symbol",  "class_", "isotope_mass", "name", "number"  };

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

set<ThermoFun::ElementKey> ElementData_::getElementsList( const string& id )
{
  set<ElementKey> elements;
  string jsonrecord = getJsonRecordVertex(id);

  ElementKey elem("",0,0);
  auto node =  jsonio::unpackJson(jsonrecord);
  elem.fromElementNode( node->field("properties") );
  elements.insert(elem);
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



}
