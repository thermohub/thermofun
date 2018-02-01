#include "boost/shared_ptr.hpp"
#include "ThermoSetData.h"

using namespace jsonio;

namespace ThermoFun {

const string datsetQuery                    = "{\"_label\": \"thermodataset\" }";
const vector<string> datsetFieldPaths       = {"properties.symbol","properties.name", "properties.type","_id"};
const vector<string> datsetColumnHeaders    = {"symbol", "name", "type"};
const vector<string> datsetDataNames        = {"symbol", "name", "type", "_id"};

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

set<ElementKey> ThermoSetData::getElementsList( const string& idSubstance )
{

}

ValuesTable ThermoSetData::loadRecordsValues( const string& aquery,
                    int sourcetdb, const vector<ElementKey>& elements )
{

}

ValuesTable ThermoSetData::loadRecordsValues( const string& idReactionSet )
{

}

auto ThermoSetData::idRecordFromSymbol (const string &symbol) -> string
{
    auto query = getQuery();
    addFieldsToQuery(query , { make_pair( string("properties.symbol"), symbol) } );
    pimpl->valuesTable = getDB()->loadRecords(query, getDataFieldPaths());
    if (pimpl->valuesTable.size() == 1)
        return pimpl->valuesTable[0][getDataName_DataIndex()["_id"]];
    else return "";
}

}
