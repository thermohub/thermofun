#include <algorithm>
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
                                           "properties.name", "properties.number", "_id" };
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

void ElementData_::updateTableByElementsList( jsonio::ValuesTable& dataQueryMatr, const vector<ElementKey>& elements )
{
    if( !elements.empty() )
    {
        jsonio::ValuesTable dataMatr;
        for (const auto& subitem : dataQueryMatr)
        {
         ElementKey elkey( subitem[getDataName_DataIndex()["symbol"]],
                           subitem[getDataName_DataIndex()["class_"]],
                           subitem[getDataName_DataIndex()["isotope_mass"]]  );
         if( std::find( elements.begin(), elements.end(), elkey) != elements.end() )
              dataMatr.push_back(subitem);
        }
        dataQueryMatr = move(dataMatr);
    }
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
    ValuesTable elemQueryMatr = getDB()->downloadDocuments(query, fields);

    // get record by elements list
    updateTableByElementsList( elemQueryMatr, elements );
    pimpl->valuesTable = elemQueryMatr;
    return    elemQueryMatr;
}

ValuesTable ElementData_::loadRecordsValues( const string& idrcset )
{
    // extract sourcetdb and elements list from
    string AQLreq = "FOR u IN reactionsets\n"
                   "FILTER u._id == '";
           AQLreq += idrcset;
           AQLreq +=  "'\n RETURN { \"elements\": u.properties.elements, \"sourcetdb\":u.properties.sourcetdb}";

    DBQueryData query( AQLreq, DBQueryData::qAQL );
    ValuesTable resMatr =  getDB()->downloadDocuments( query, { "elements", "sourcetdb" });

    // read elements by sourcetdb and elements list
    if( resMatr.size() > 0)
    {
        set<ElementKey> elementset;
        ElementsKeysFromJson( resMatr[0][0], elementset );
        vector<ElementKey> elements(elementset.begin(), elementset.end());
        int sourcetdb = ElementKey::index_from_map( resMatr[0][1] );

        return loadRecordsValues( jsonio::emptyQuery, sourcetdb, elements );
    }
    return   ValuesTable();
}


// Extract data defined sourcetdbs and elements
std::vector<ElementKey> ElementData_::selectGiven( const std::vector<int>& sourcetdbs, const std::vector<ElementKey>& elements )
{
    string AQLreq = "FOR u IN elements \n"
                    "  FILTER u.properties.sourcetdb IN @sourcetdbs\n"
                    "  SORT u.properties.symbol ";

    // generate bind values
    shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
    auto arr = domdata->appendArray( "sourcetdbs");
    sourceTDB_from_indexes( sourcetdbs, arr );
    // make query
    DBQueryData query( AQLreq, DBQueryData::qAQL );
    query.setBindVars( domdata.get() );
    query.setQueryFields( makeQueryFields() );

    ValuesTable elmQueryMatr = getDB()->downloadDocuments(query, getDataNames());

    updateTableByElementsList( elmQueryMatr, elements );

    vector<ElementKey> elmKeys;
    for (const auto& elitem : elmQueryMatr)
    {
        elmKeys.push_back( ElementKey( elitem[getDataName_DataIndex()["symbol"]],
                                       elitem[getDataName_DataIndex()["class_"]],
                                       elitem[getDataName_DataIndex()["isotope_mass"]] ));
    }

    pimpl->valuesTable =          move(elmQueryMatr);
    return                elmKeys;
}


vector<string> ElementData_::selectGiven( const vector<string>& idThermoDataSets, bool /*unique*/ )
{
    string qrAQL = "FOR vertex IN " + vectorToJson( idThermoDataSets);
           qrAQL += "\n  FOR v,e  IN 1..5 INBOUND vertex \n";
           qrAQL +=  " basis";
           qrAQL +=  "\n  FILTER v._label == 'element' ";
           qrAQL +=  "\n  SORT v.properties.symbol ";
           qrAQL +=  DBQueryData::generateReturn( true, makeQueryFields(), "v");
    DBQueryData query( qrAQL, DBQueryData::qAQL );
    ValuesTable resMatr =  getDB()->downloadDocuments( query, getDataNames());

    vector<string> elmSymbols;
    for (const auto& subitem : resMatr)
       elmSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

    setDefaultLevelForReactionDefinedSubst(resMatr);
    pimpl->valuesTable = move(resMatr);
    return elmSymbols;
}

}
