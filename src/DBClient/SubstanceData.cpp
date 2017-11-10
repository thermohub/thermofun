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
    bsonio::ValuesTable valuesTable;

    Impl( )
    {
    }

};

SubstanceData::SubstanceData()
    : ThermoDataAbstract( "VertexSubstance", substQuery, substFieldPaths, substColumnHeaders, substDataNames), pimpl(new Impl())
{ }

auto SubstanceData::operator=(SubstanceData other) -> SubstanceData&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

SubstanceData::~SubstanceData()
{ }

auto SubstanceData::queryInEdgesDefines(string idSubst, vector<string> queryFields,  string level) -> vector<string>
{
    return queryInEdgesDefines_(idSubst, queryFields, level);
}

auto SubstanceData::definesReactionSymbol(string idSubst, string level) -> std::string
{
    return definesReactionSymbol_(idSubst, level);
}

set<ElementKey> SubstanceData::getElementsList( const string& idSubstance )
{
  string formula;
  getDB()->GetRecord( (idSubstance+":").c_str() );
  getDB()->getValue( getDataPath()["formula"], formula);
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
      addFieldsToQuery( query, { make_pair( string(getDataPath()["sourcetdb"]), to_string(sourcetdb)) } );

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
          string formula = subitem[getDataIndex()["formula"]];
          if( testElementsFormula( formula, elements)  )
               substMatr.push_back(subitem);
         }
    }
    setDefaultLevelForReactionDefinedSubst(substMatr);
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


}
