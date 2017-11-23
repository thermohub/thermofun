#include "SubstanceData.h"

using namespace bsonio;

namespace ThermoFun {

const string substQuery = "{\"_label\": \"substance\" }";
const vector<string> substFieldPaths =
   { "properties.symbol","properties.name","properties.formula","_id", "properties.class_", "properties.sourcetdb"};
const vector<string> substColumnHeaders = { "symbol", "name", "formula" };
const vector<string> substDataNames  = { "symbol", "name", "formula", "_id", "class_", "sourcetdb" };

struct SubstanceData_::Impl
{
    bsonio::ValuesTable valuesTable;

    Impl( )
    {
    }

};

SubstanceData_::SubstanceData_()
    : AbstractData( "VertexSubstance", substQuery, substFieldPaths, substColumnHeaders, substDataNames), pimpl(new Impl())
{ }

SubstanceData_::SubstanceData_(const SubstanceData_& other)
: AbstractData(other), pimpl(new Impl(*other.pimpl))
{}

auto SubstanceData_::operator=(SubstanceData_ other) -> SubstanceData_&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

SubstanceData_::~SubstanceData_()
{ }

auto SubstanceData_::queryInEdgesDefines(string idSubst, vector<string> queryFields,  string level) -> vector<string>
{
    return queryInEdgesDefines_(idSubst, queryFields, level);
}

auto SubstanceData_::definesReactionSymbol(string idSubst, string level) -> std::string
{
    return definesReactionSymbol_(idSubst, level);
}

set<ElementKey> SubstanceData_::getElementsList( const string& idSubstance )
{
  string formula;
  getDB()->GetRecord( (idSubstance+":").c_str() );
  getDB()->getValue( getDataName_DataFieldPath()["formula"], formula);
  FormulaToken parser(formula);
  return parser.getElements();
}

ValuesTable SubstanceData_::loadRecordsValues( const string& aquery,
                    int sourcetdb, const vector<ElementKey>& elements )
{
    // get records by query
    string query = aquery;
    if (query.empty())
       query = getQuery();
    if (!elements.empty())
      addFieldsToQuery( query, { make_pair( string(getDataName_DataFieldPath()["sourcetdb"]), to_string(sourcetdb)) } );

    ValuesTable substQueryMatr = getDB()->loadRecords(query, getDataFieldPaths());

    // get record by elements list
    ValuesTable substMatr;
    if (elements.empty())
    {
       substMatr = substQueryMatr;
    }
    else
     {  for (const auto& subitem : substQueryMatr)
        {
          string formula = subitem[getDataName_DataIndex()["formula"]];
          if (testElementsFormula(formula, elements))
               substMatr.push_back(subitem);
         }
    }
    setDefaultLevelForReactionDefinedSubst(substMatr);
    pimpl->valuesTable =          substMatr;
    return                substMatr;
}

ValuesTable SubstanceData_::loadRecordsValues( const string& idReactionSet )
{
    auto subIds = getOutVertexIds("product", idReactionSet);
    auto subIds2 = getOutVertexIds("master", idReactionSet);
    subIds.insert(subIds.end(), subIds2.begin(), subIds2.end());

    ValuesTable substMatr = getDB()->loadRecords(subIds, getDataFieldPaths());
    setDefaultLevelForReactionDefinedSubst(substMatr);
    pimpl->valuesTable = substMatr;
    return substMatr;
}

auto SubstanceData_::querySolvents(int sourcetdb) -> vector<vector<string>>
{
  string qrJson = "{ \"_label\" : \"substance\", \"$and\" : [{\"properties.class_\" : 3}]}";
  addFieldsToQuery( qrJson, { make_pair( string("properties.sourcetdb"), to_string(sourcetdb)) } );

  ValuesTable solventMatr = getDB()->loadRecords(qrJson, getDataFieldPaths());
  return solventMatr;
}


}
