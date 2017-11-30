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


struct ReactionData_::Impl
{
    bsonio::ValuesTable valuesTable;

    Impl( )
    {
    }

};

ReactionData_::ReactionData_(const bsonio::TDataBase* dbconnect)
    : AbstractData(dbconnect, "VertexReaction", reactQuery, reactFieldPaths,
                   reactColumnHeaders, reactDataNames), pimpl(new Impl())
{
}

ReactionData_::ReactionData_(const ReactionData_& other)
: AbstractData(other), pimpl(new Impl(*other.pimpl))
{}

auto ReactionData_::operator=(ReactionData_ other) -> ReactionData_ &
{
    pimpl = std::move(other.pimpl);
    return *this;
}

ReactionData_::~ReactionData_()
{
}

auto ReactionData_::queryInEdgesTakes(string idReact, vector<string> queryFields) -> vector<string>
{
    return queryInEdgesTakes_(idReact, queryFields);
}

auto ReactionData_::reactantsCoeff(string idReact) -> std::map<std::string, double>
{
    return reactantsCoeff_(idReact);
}

bsonio::ValuesTable ReactionData_::loadRecordsValues(const string &aquery,
                                                    int sourcetdb, const vector<ElementKey> &elements)
{
    // get records by query
    string query = aquery;
    if (query.empty())
        query = getQuery();
    if (!elements.empty())
        addFieldsToQuery(query, {make_pair(string("properties.sourcetdb"), to_string(sourcetdb))});
    ValuesTable reactQueryMatr = getDB()->loadRecords(query, getDataFieldPaths());

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
            string idreac = subitem[getDataName_DataIndex()["_id"]];
            if (testElements(idreac, elements))
                reactMatr.push_back(subitem);
        }
    }
    setDefaultLevelForReactionDefinedSubst(reactMatr);
    pimpl->valuesTable = reactMatr;
    return reactMatr;
}

bsonio::ValuesTable ReactionData_::loadRecordsValues( const string& idReactionSet )
{
    auto reIds = getInVertexIds( "prodreac", idReactionSet );
    ValuesTable reactMatr = getDB()->loadRecords(reIds, getDataFieldPaths());
    setDefaultLevelForReactionDefinedSubst(reactMatr);
    pimpl->valuesTable = reactMatr;
    return reactMatr;
}

bool ReactionData_::testElements(const string &idReaction,
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

vector<string> ReactionData_::getReactantsFormulas(const string &idReaction)
{
    vector<string> formulas;
    string idSubst, formSubst;
    bson record;

    // select all EdgeTakes for reaction
    vector<string> _resultDataEdge = queryInEdgesTakes(idReaction, {"_outV"});

    // for all substances
    for (auto rec : _resultDataEdge)
    {
        idSubst = bsonio::extractStringField("_outV", rec);
        record = getJsonBsonRecordVertex(idSubst+":").second;
        bsonio::bson_to_key( record.data, "properties.formula", formSubst);
        formulas.push_back(formSubst);
    }
    return formulas;
}

set<ElementKey> ReactionData_::getElementsList(const string &idReaction)
{
    set<ElementKey> elements; bson obj;
    obj = getJsonBsonRecordVertex(idReaction+":").second;
    ElementsFromBsonArray("properties.elements", obj.data, elements);
//    bson_destroy(&obj);

    // if user fogot insert elements property
    if (elements.empty())
    {
        vector<string> formulalst = getReactantsFormulas(idReaction);
        elements = ChemicalFormula::extractElements(formulalst);
    }
    return elements;
}

void ReactionData_::resetRecordElements(const string& idReact )
{
    string _id;
    try{
        getDB()->GetRecord(idReact.c_str());
        getDB()->getValue("_id",_id);

        vector<string> formulalst = getReactantsFormulas( _id );
        set<ThermoFun::ElementKey> elements = ThermoFun::ChemicalFormula::extractElements(formulalst );

        string elementsJsonArray = ElementsToJson( elements );
        getDB()->setValue("properties.elements",elementsJsonArray);
        getDB()->SaveCurrent( true, true  );
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

vector<string> ReactionData_::getKeys(string symbol, string sourcetdb)
{
    string queryJson;
    queryJson = "{'_type': 'vertex', '_label': 'reaction', 'properties.symbol': '";
    queryJson += symbol;
    queryJson += "',  'properties.sourcetdb': ";
    queryJson += sourcetdb;
    queryJson += " }";
    return getDB()->getKeysByQuery(queryJson);
}

bool ReactionData_::checkReactSymbolLevel (string sourcetdb, string &symbol, string &level)
{
    vector<int> levels;
    string queryJson; ValuesTable levelQueryMatr;
    string reactSymbol = symbol;
    vector<string> reactKeys = getKeys(symbol, sourcetdb);
    level = "0";
    while (reactKeys.size() > 0)
    {
        levels.clear();

        for (auto key_: reactKeys)
        {
            strip_all( key_, ":" );
            queryJson = "{'_type': 'edge', '_label': 'defines', '_outV': '";
            queryJson += key_;
            queryJson += "'}";

            // level of the defines edge
            levelQueryMatr = getDB_edgeAccessMode()->loadRecords( queryJson, {"properties.level"} );
            if (levelQueryMatr.size()>0)
                levels.push_back(std::stoi(levelQueryMatr[0][0]));
            else
                levels.push_back(std::stoi(level));
        }

        if (levels.size()>0)
        {
            level  = std::to_string((*std::max_element(levels.begin(), levels.end())+1));
            symbol = reactSymbol + "_"+level;
        }

        reactKeys = getKeys(symbol, sourcetdb);
    }

    if (symbol == reactSymbol)
        return false;
    else
        return true;
}

}
