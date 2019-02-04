#include "ThermoSetData.h"
#include "jsonio/jsondomfree.h"
#include <algorithm>

using namespace std;
using namespace jsonio;

namespace ThermoFun {

//const DBQueryData datsetQuery( "{\"_label\": \"thermodataset\" }", DBQueryData::qTemplate );
const DBQueryData datsetQuery("FOR u  IN thermodatasets ", DBQueryData::qAQL);
const vector<string> datsetFieldPaths       = {"properties.symbol","properties.name", "properties.stype","_id"};
const vector<string> datsetColumnHeaders    = {"symbol", "name", "stype"};
const vector<string> datsetDataNames        = {"symbol", "name", "stype", "_id"};

struct ThermoSetData::Impl
{
    ValuesTable valuesTable;

    Impl( )
    {
    }

};

ThermoSetData::ThermoSetData( const TDataBase* adbconnect )
    : AbstractData( adbconnect, "VertexThermoDataSet", datsetQuery,
                    datsetFieldPaths, datsetColumnHeaders, datsetDataNames), pimpl(new Impl())
{ }

ThermoSetData::ThermoSetData(const ThermoSetData& other)
: AbstractData(other), pimpl(new Impl(*other.pimpl))
{}

auto ThermoSetData::operator=(ThermoSetData other) -> ThermoSetData&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

ThermoSetData::~ThermoSetData()
{}

const jsonio::ValuesTable&  ThermoSetData::getValuesTable()
{
    return pimpl->valuesTable;
}

set<ElementKey> ThermoSetData::getElementsList( const string& idthermo )
{
    set<ElementKey> elements;
    string jsonrecord = getJsonRecordVertex(idthermo);
    auto domdata = jsonio::unpackJson( jsonrecord );

    // query elements
    vector<string> jElements;
    string qrAQL = "FOR v,e  IN 1..1 INBOUND '" + idthermo + "' \n";
           qrAQL +=  "basis";
           qrAQL +=  "\n  FILTER v._label == 'element' ";
           qrAQL += "\nRETURN DISTINCT v";

    jElements =  getDB()->runQuery( DBQueryData( qrAQL, DBQueryData::qAQL ) );

    ElementKey elem("",0,0);
    for (auto jel : jElements)
    {
        auto node =  jsonio::unpackJson(jel);
        elem.fromElementNode( node->field("properties"));
        elements.insert(elem);
    }
    //ElementsFromJsonDomArray("properties.elements", domdata.get(), elements);

    // if user fogot tnsert elements property
    if( elements.empty() )
    {
        vector<string> formulalst = getSubstanceFormulas( idthermo );
        elements = ThermoFun::ChemicalFormula::extractElements(formulalst );
    }
    return elements;
}

vector<string> ThermoSetData::getSubstanceFormulas( const string& idthermo )
{
    string qrAQL = "FOR v,e  IN 1..5 INBOUND '" + idthermo + "' \n";
           qrAQL +=  ThermoDataSetQueryEdges;
           qrAQL +=  "\n  FILTER v._label == 'substance' ";
           qrAQL += "\nRETURN DISTINCT v.properties.formula";

    vector<string> formulas = getDB()->runQuery( DBQueryData( qrAQL, DBQueryData::qAQL ) );
    return formulas;
}

ValuesTable ThermoSetData::loadRecordsValues( const DBQueryData& aquery,
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
    //?? if (!elements.empty())
    //??  addFieldsToQueryAQL( query, { make_pair( string(getDataName_DataFieldPath()["sourcetdb"]), to_string(sourcetdb)) } );

    ValuesTable substQueryMatr = getDB()->downloadDocuments(query, fields);

    // get record by elements list
    //updateTableByElementsList( substQueryMatr, elements );
    pimpl->valuesTable = substQueryMatr;
    return   substQueryMatr;
}

ValuesTable ThermoSetData::loadRecordsValues( const string& idReactionSet )
{
    ValuesTable substQueryMatr;
    return   substQueryMatr;
}

auto ThermoSetData::idRecordFromSymbol (const string &symbol) -> string
{
    string qrJson = "FOR u  IN thermodatasets ";
           qrJson += "FILTER u.properties.symbol == \""+symbol+"\" ";
           qrJson += "RETURN u._id";

    vector<string> ids = getDB()->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ) );
    for (auto &i:ids)
        while (std::find(i.begin(), i.end(), '"') != i.end())
            i.erase(std::find(i.begin(), i.end(), '"'));
    if ( ids.size() == 1)
        return ids[0];
    else return "";
}

vector<string> ThermoSetData::selectGiven( const vector<string>& idThermoDataSets, bool  )
{
    auto  resMatr = getDB()->downloadDocuments(idThermoDataSets, getDataFieldPaths());

    vector<string> thermoSymbols;
    for (const auto& subitem : resMatr)
      thermoSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

    pimpl->valuesTable = move(resMatr);
    return thermoSymbols;
}


// Visit  all linked Vertexes
void ThermoSetData::traverceVertexes( const string& idThermoDataSet, jsonio::GraphElementFunction afunc )
{
    string qrAQL = "FOR v,e  IN 1..5 INBOUND '" + idThermoDataSet + "' \n";
           qrAQL +=  ThermoDataSetQueryEdges;
           qrAQL +=  "\n  SORT v._label ";
           qrAQL +=  " \n RETURN DISTINCT  v";

    DBQueryData query( qrAQL, DBQueryData::qAQL );
    vector<string> resultsQuery = getDB()->runQuery( query );

    for( auto result: resultsQuery )
        afunc( true, result );
}

// Visit  all linked Edges
void ThermoSetData::traverceEdges( const string& idThermoDataSet, jsonio::GraphElementFunction afunc )
{
    string qrAQL = "FOR v,e  IN 1..5 INBOUND '" + idThermoDataSet + "' \n";
           qrAQL +=  ThermoDataSetQueryEdges;
           qrAQL +=  "\n  SORT e._label ";
           qrAQL +=  " \n RETURN DISTINCT  e";

    DBQueryData query( qrAQL, DBQueryData::qAQL );
    vector<string> resultsQuery = getDB()->runQuery( query );

    for( auto result: resultsQuery )
        afunc( false, result );
}



}
