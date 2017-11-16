#include "ReactionData.h"

// C++ includes
#include <functional>

// ThermoFun includes
#include "../OptimizationUtils.h"

using namespace bsonio;

namespace ThermoFun
{

const string reactQuery = "{\"_label\": \"reaction\" }";
const vector<string> reactFieldPaths =
    {"properties.symbol", "properties.name", "properties.equation", "_id", "properties.level", "properties.sourcetdb"};
const vector<string> reactColumnHeaders = {"symbol", "name", "equation"};
const vector<string> reactDataNames = {"symbol", "name", "equation", "_id", "level", "sourcetdb"};

using QueryVertexReaction  = std::function<string(string, vector<string>)>;


struct ReactionData::Impl
{
    bsonio::ValuesTable valuesTable;
    int t = 0;

    Impl( )
    {
    }

};

ReactionData::ReactionData()
    : ThermoDataAbstract("VertexReaction", reactQuery, reactFieldPaths, reactColumnHeaders, reactDataNames), pimpl(new Impl())
{
}

//ReactionData::ReactionData(const ReactionData& other)
// : pimpl(new Impl(*other.pimpl))
//{}

auto ReactionData::operator=(ReactionData other) -> ReactionData &
{
    pimpl = std::move(other.pimpl);
    return *this;
}

ReactionData::~ReactionData()
{
}

auto ReactionData::queryInEdgesTakes(string idReact, vector<string> queryFields) -> vector<string>
{
    return queryInEdgesTakes_(idReact, queryFields);
}

auto ReactionData::reactantsCoeff(string idReact) -> std::map<std::string, double>
{
    return reactantsCoeff_(idReact);
}

bsonio::ValuesTable ReactionData::loadRecordsValues(const string &aquery,
                                                    int sourcetdb, const vector<ElementKey> &elements)
{
    // get records by query
    string query = aquery;
    if (query.empty())
        query = getQuery();
    if (!elements.empty())
        addFieldsToQuery(query, {make_pair(string("properties.sourcetdb"), to_string(sourcetdb))});
    ValuesTable reactQueryMatr = getDB()->loadRecords(query, getFieldPaths());

    // get record by elements list
    ValuesTable reactMatr;
    if (elements.empty())
    {
        reactMatr = reactQueryMatr;
    }
    else
    {
        for (const auto &subitem : reactQueryMatr)
        {
            string idreac = subitem[getDataIndex()["_id"]];
            if (testElements(idreac, elements))
                reactMatr.push_back(subitem);
        }
    }
    setDefaultLevelForReactionDefinedSubst(reactMatr);
    pimpl->valuesTable = reactMatr;
        pimpl->t = 1;
    return reactMatr;
}

bool ReactionData::testElements(const string &idReaction,
                                const vector<ElementKey> &elements)
{
    set<ElementKey> reactelements = getElementsList(idReaction);

    /* if user fogot insert elements property
   if( reactelements.empty() )
   {
       vector<string> formulalst = getSubstanceFormulas( idReaction );
       reactelements = elements::ChemicalFormula::extractElements(formulalst );
   }*/

    for (auto formelm : reactelements)
    {
        auto itr = elements.begin();
        while (itr != elements.end())
        {
            if (formelm == *itr)
                break;
            itr++;
        }
        if (itr == elements.end())
            return false;
    }
    return true;
}

vector<string> ReactionData::getReactantsFormulas(const string &idReaction)
{
    vector<string> formulas;
    string idSub, formSub;

    // select all EdgeTakes for reaction
    string queryJson = "{'_type': 'edge', '_label': 'takes', '_inV': '" + idReaction + "' }";
    vector<string> _queryFields = {"_outV"};
    vector<string> _resultData;
    getDB()->runQuery(queryJson, _queryFields, _resultData);

    // for all substances
    for (auto rec : _resultData)
    {
        idSub = bsonio::extractStringField("_outV", rec);
        getDB_fullAccessMode()->GetRecord((idSub + ":").c_str());
        getDB_fullAccessMode()->getValue("properties.formula", formSub);
        formulas.push_back(formSub);
    }
    return formulas;
}

set<ElementKey> ReactionData::getElementsList(const string &idReaction)
{
    set<ElementKey> elements;
    getDB()->GetRecord((idReaction + ":").c_str());
    bson obj;
    getDB()->GetBson(&obj);
    ElementsFromBsonArray("properties.elements", obj.data, elements);
    bson_destroy(&obj);

    // if user fogot insert elements property
    if (elements.empty())
    {
        vector<string> formulalst = getReactantsFormulas(idReaction);
        elements = ChemicalFormula::extractElements(formulalst);
    }
    return elements;
}
}
