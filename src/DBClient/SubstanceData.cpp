#include "SubstanceData.h"

using namespace bsonio;

namespace ThermoFun {


const string substQuery = "{\"_label\": \"substance\" }";
const vector<string> substFieldPaths =
   { "properties.symbol","properties.name","properties.formula","_id", "properties.class_", "properties.sourcetdb"};
const vector<string> substColumnHeaders = { "symbol", "name", "formula" };
const vector<string> substDataNames  = { "symbol", "name", "formula", "_id", "class_", "sourcetdb" };

struct SubstanceData::Impl
{

    std::map<std::string, int> substDataIndex;
    std::map<std::string, std::string> substDataPath;

    std::map<std::string, std::string> symbolLevel;

    bsonio::ValuesTable valuesTable;


    Impl( )
    { }

};

SubstanceData::SubstanceData( boost::shared_ptr<bsonio::TDBGraph> dbgraph )
    : ThermoDataAbstract( "VertexSubstance", substQuery, substFieldPaths, substColumnHeaders, substDataNames), pimpl(new Impl())
{
    setDB(dbgraph);
    resetSubstDataPathIndex();
}

void SubstanceData::resetSubstDataPathIndex()
{
    pimpl->substDataIndex.clear(); pimpl->substDataPath.clear();
    for (uint i = 0; i<getDataNames().size(); i++)
    {
        pimpl->substDataIndex[getDataNames()[i]] = i;
        pimpl->substDataPath[getDataNames()[i]] = getFieldPaths()[i];
    }
//    updateDBClient();
}


map<ElementKey, double> SubstanceData::getElementsList( const string& idrec )
{
  string formula;
  getDB()->GetRecord( (idrec+":").c_str() );
  getDB()->getValue(  pimpl->substDataPath["formula"], formula);
  FormulaToken parser(formula);
  return parser.getElements();
}

bsonio::ValuesTable SubstanceData::loadRecordsValues( const string& aquery,
                    int sourcetdb, const vector<ElementKey>& elements )
{
    // get records by query
    string query = aquery;
    if( query.empty() )
       query = getQuery();
    if( !elements.empty() )
      addFieldsToQuery( query, { make_pair( string(pimpl->substDataPath["sourcetdb"]), to_string(sourcetdb)) } );

    ValuesTable substQueryMatr = getDB()->loadRecords( query, getFieldPaths() );

    // get record by elements list
    ValuesTable substMatr;
    if( elements.empty() )
    {
       substMatr = substQueryMatr;
    }
    else
     {  for( const auto& subitem : substQueryMatr )
        {
          string formula = subitem[pimpl->substDataIndex["formula"]];
          if( testElementsFormula( formula, elements)  )
               substMatr.push_back(subitem);
         }
    }
    setDefaultSymbolLevel(substMatr);
    pimpl->valuesTable =          substMatr;
    return                substMatr;
}

//bsonio::ValuesTable SubstanceData::loadRecordsValues( const string& idReactionSet )
//{
//    auto subIds = getOutVertexIds( "product", idReactionSet );
//    auto subIds2 = getOutVertexIds( "master", idReactionSet );
//    subIds.insert(  subIds.end(), subIds2.begin(), subIds2.end() );

//    ValuesTable substMatr = getDB()->loadRecords(subIds, getFieldPaths());
//    setDefaultSymbolLevel(substMatr);
//    pimpl->valuesTable = substMatr;
//    return substMatr;
//}

void SubstanceData::setDefaultSymbolLevel(bsonio::ValuesTable valuesTable)
{
    for( const auto& subitem : valuesTable )
    {
        pimpl->symbolLevel[subitem[pimpl->substDataIndex["symbol"]]] = "0";
    }
}

}
