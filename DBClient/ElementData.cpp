#include <algorithm>
#include "ElementData.h"
#include "jsonio/io_settings.h"
#include "jsonio/jsondomfree.h"
#include "sourcetdb.h"

// ThermoFun includes
#ifdef FROM_SRC
#include "../src/Common/formuladata.h"
#include "../src/Element.h"
#include "../src/Database.h"
#include "../src/Common/ParseJsonToData.h"
#else
#include "thermofun/Common/formuladata.h"
#include "thermofun/Element.h"
#include "thermofun/Common/ParseJsonToData.h"
#include "thermofun/Database.h"
#endif

using namespace std;
using namespace jsonio;

namespace ThermoFun
{

//const DBQueryData reactQuery( "{\"_label\": \"element\" }", DBQueryData:: qTemplate );
const DBQueryData elementQuery("FOR u  IN elements ", DBQueryData::qAQL);
const vector<string> elementFieldPaths = { "properties.symbol", "properties.name", "properties.number", "properties.sourcetdb", "properties.class_", "properties.isotope_mass", "_id" };
const vector<string> elementDataNames = {"symbol",               "name",                "number",           "sourcetdb",            "class_",                    "mass", "_id" }; // should have the same size as FieldPaths !!
const vector<string> elementColumnHeaders = { "symbol",          "name",                "number"/*,    "sourcetdb" */ };

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
  elem = fromElementNode( node->field("properties") );
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
                           stoi(subitem[getDataName_DataIndex()["class_"]]),
                           stoi(subitem[getDataName_DataIndex()["isotope_mass"]])  );
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
        int sourcetdb = index_from_map( resMatr[0][1] );

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
                                       stoi(elitem[getDataName_DataIndex()["class_"]]),
                                       stoi(elitem[getDataName_DataIndex()["isotope_mass"]]) ));
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

// Reads data from JsonDom (fromm real record)
auto fromElementNode( const jsonio::JsonDom *object ) -> ElementKey
{
    std::string symbol; int isotope, class_;
    if(!object->findValue( "symbol", symbol ) )
        jsonio::jsonioErr( "ElementKey: ", "Undefined symbol.");
    if(!object->findValue( "isotope_mass", isotope ) )
        isotope = 0;
    std::map<std::string, int> map_;
    if(!object->findObject("class_", map_ ) ||  map_.size() < 1 )
        class_ = 0;
    else
        class_ = atoi(map_.begin()->first.c_str());
    return ElementKey(symbol,class_,isotope);
}

// Reads data from JsonDom (only key)
auto fromKeyNode( const jsonio::JsonDom *object ) -> ElementKey
{
    std::string symbol; int isotope, class_;
    if(!object->findValue( "symbol", symbol ) )
        jsonio::jsonioErr( "ElementKey: ", "Undefined symbol.");
    if(!object->findValue( "isotope_mass", isotope ) )
        isotope = 0;
    if(!object->findValue("class_", class_ ) )
        class_ = 0;
    return ElementKey(symbol,class_,isotope);
}

// Work only with internal elements list (wihout map)
void ElementsKeysFromJsonDom( const jsonio::JsonDom *object, set<ElementKey>& elements )
{
    ElementKey elem("",0,0);
    size_t objsize = object->getChildrenCount();

    for( size_t ii=0; ii<objsize; ii++ )
    {
       auto childobj = object->getChild( ii);
       if( childobj->getType() != jsonio::JSON_OBJECT )
            continue;
        elem = fromKeyNode(childobj);
        elements.insert(elem);
    }
 }

bool ElementsKeysFromJson( const string elmsjson, set<ElementKey>& elements )
{
    try{
        auto arrobject = jsonio::unpackJson( elmsjson );
        elements.clear();
        ElementsKeysFromJsonDom( arrobject.get(), elements );
        return true;
    }
    catch(...)
      {
        return false;
      }
}

int index_from_map(std::string map)
{
    if( map.empty() )
       return 0;
    auto first = map.find("\"");
    jsonio::jsonioErrIf( first == string::npos, map, "Illegal class value.");
    auto second = map.find("\"", first+1);
    jsonio::jsonioErrIf( second == string::npos, map, "Illegal class value.");
    string strNew = map.substr (first+1,second-(first+1));
    return stoi(strNew);
}

bool ElementsKeysFromJsonDomArray( const string& keypath, const jsonio::JsonDom *object, set<ElementKey>& elements )
{
    elements.clear();
    auto elmobj = object->field(keypath);
    if(elmobj==nullptr)
      return false;
    ElementsKeysFromJsonDom( elmobj, elements );
    return true;
}

auto ElementKeyFromDB( jsonio::TDBVertexDocument* elementDB ) -> ElementKey
{
    std::string symbol; int isotope_, class_;
    fromElementNode(elementDB->getDom());
    elementDB->getValue( "properties.symbol" , symbol );
    std::string class_str;
    elementDB->getValue( "properties.class_" , class_str );
    class_ = index_from_map(class_str);
    elementDB->getValue( "properties.isotope_mass" , isotope_ );
    return ElementKey(symbol,class_,isotope_);
}

// Writes data to json (only key)
void toKeyNode(  Element el, jsonio::JsonDom *object )
{
    object->appendString( "symbol", el.name() );
    if( el.isotopeMass() != 0 )
        object->appendInt( "isotope_mass", el.isotopeMass() );
    if( el.class_()!= 0 )
        object->appendInt( "class_", el.class_() );
}

void ElementsKeysToJsonDom( jsonio::JsonDom *object, const set<ElementKey>& elements )
{
  int ndx=0;
  for( auto el: elements)
  {
      auto objel = object->appendObject(to_string(ndx++));
      toKeyNode(elementKeyToElement(el), objel);
  }
}

shared_ptr<jsonio::JsonDomFree> ElementsKeysToJsonArray( const set<ElementKey>& elements )
{
    shared_ptr<jsonio::JsonDomFree> domdata(jsonio::JsonDomFree::newArray());
    ElementsKeysToJsonDom( domdata.get(), elements );
    return domdata;
}

string ElementsKeysToJson( const set<ElementKey>& elements )
{
  auto domarray = ElementsKeysToJsonArray( elements );
  string elmsjson;
  printNodeToJson( elmsjson, domarray.get() );
  return elmsjson;
}

void setDBElements( jsonio::TDBVertexDocument* elementDB, const jsonio::DBQueryData& query )
{
    vector<string> resultData = elementDB->runQuery( query );

    ElementsMap elements_map;
    for(uint ii=0; ii<resultData.size(); ii++ )
    {
//       cout << resultData[ii] << endl;
      auto e = parseElement(resultData[ii]);
      elements_map[e.symbol()] = e;
    }
    if (elements_map.size()>0)
        ChemicalFormula::setDBElements( elements_map );
}

}
