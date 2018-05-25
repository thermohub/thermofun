#include "ReactionData.h"

// C++ includes
#include <functional>
#include <algorithm>
#include "jsonio/jsondomfree.h"

// ThermoFun includes
#include "OptimizationUtils.h"

using namespace jsonio;

namespace ThermoFun
{

//const DBQueryData reactQuery("{\"_label\": \"reaction\" }", DBQueryData::qTemplate);
const DBQueryData reactQuery("FOR u  IN reactions ", DBQueryData::qAQL);
const vector<string> reactFieldPaths =
    {"properties.symbol", "properties.name", "properties.equation", "_id", "properties.level", "properties.sourcetdb"};
const vector<string> reactColumnHeaders = {"symbol", "name", "equation"};
const vector<string> reactDataNames = {"symbol", "name", "equation", "_id", "level", "sourcetdb"};

using QueryVertexReaction  = std::function<string(string, vector<string>)>;


struct ReactionData_::Impl
{
    ValuesTable valuesTable;

    Impl( )
    {
    }

};

ReactionData_::ReactionData_(const TDataBase* dbconnect)
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

void ReactionData_::updateTableByElementsList( ValuesTable& reactQueryMatr, const vector<ElementKey>& elements )
{
    if( !elements.empty() )
    {
        ValuesTable reactMatr;
        for (const auto& subitem : reactQueryMatr)
        {
            string idreac = subitem[getDataName_DataIndex()["_id"]];
            if (testElements(idreac, elements))
                reactMatr.push_back(subitem);
        }
        reactQueryMatr = move(reactMatr);
    }
 }

// return all record, no only fields if not default
ValuesTable ReactionData_::loadRecordsValues(const DBQueryData& aquery,
                                  int sourcetdb, const vector<ElementKey> &elements)
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
    //if (!elements.empty())
        addFieldsToQueryAQL(query, {make_pair(string(getDataName_DataFieldPath()["sourcetdb"]), to_string(sourcetdb))});
    ValuesTable reactQueryMatr = getDB()->loadRecords(query, fields);

    // get record by elements list
    updateTableByElementsList( reactQueryMatr, elements );
    setDefaultLevelForReactionDefinedSubst(reactQueryMatr);
    pimpl->valuesTable = reactQueryMatr;
    return move(reactQueryMatr);
}

ValuesTable ReactionData_::loadRecordsValues( const string& idReactionSet )
{
    string qrAQL = "FOR u,e  IN 1..1 INBOUND '";
           qrAQL += idReactionSet + "' \n prodreac\n";
           //qrAQL += "RETURN u";

    DBQueryData query( qrAQL, DBQueryData::qAQL );
    query.setQueryFields(makeQueryFields());
    ValuesTable reactMatr =  getDB()->loadRecords( query, getDataNames());
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
    string qrJson = "FOR v,e  IN 1..1 INBOUND '";
           qrJson += idReaction + "' \n takes\n";
           qrJson += "RETURN v.properties.formula";

    getDB()->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ),  {}, formulas);
    return formulas;
}

set<ElementKey> ReactionData_::getElementsList(const string &idReaction)
{
    set<ElementKey> elements;
    string jsonrecord = getJsonRecordVertex(idReaction+":");
    auto domdata = jsonio::unpackJson( jsonrecord );
    ElementsFromJsonDomArray("properties.elements", domdata.get(), elements);

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
    catch(jsonio_exeption& e)
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
   string qrAQL = "FOR u  IN reactions \nFILTER u.properties.symbol == '";
          qrAQL += symbol + "' and  u.properties.sourcetdb == "+ sourcetdb;
           //qrAQL += "RETURN u";

    DBQueryData query( qrAQL, DBQueryData::qAQL );
    return getDB()->getKeysByQuery( query );
}

bool ReactionData_::checkReactSymbolLevel (string sourcetdb, string &symbol, string &level)
{
    vector<int> levels;
    vector<string> levelQueryMatr;
    string reactSymbol = symbol;
    vector<string> reactKeys = getKeys(symbol, sourcetdb);
    level = "0";
    while (reactKeys.size() > 0)
    {
        levels.clear();

        for (auto key_: reactKeys)
        {
            strip_all( key_, ":" );
            string qrJson = "FOR v,e  IN 1..1 OUTBOUND '";
                   qrJson += key_ + "' \n defines\n";
                   qrJson += "RETURN e.properties.level";
            getDB()->runQuery( DBQueryData( qrJson, DBQueryData::qAQL ),  {}, levelQueryMatr);
            if (levelQueryMatr.size()>0)
                levels.push_back(std::stoi(levelQueryMatr[0]));
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

vector<string> ReactionData_::selectGiven( const vector<int>& sourcetdbs,
                   const vector<string>& substanceSymbols, bool unique )
{
    // define query string
    string AQLreq = "FOR u IN reactions\n"
                    "  FILTER u.properties.sourcetdb IN @sourcetdbs\n"
                    "  LET takessub = ( FOR v,e IN 1..1 INBOUND u._id takes  RETURN v.properties.symbol ) \n"
                    "  FILTER takessub ALL IN @substanceSymbols \n"
                    "  SORT u.properties.symbol ";
           AQLreq += DBQueryData::generateReturn(makeQueryFields());

    // generate bind values
    shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
    auto arr = domdata->appendArray( "sourcetdbs");
    for(uint ii=0; ii<sourcetdbs.size(); ii++)
        arr->appendInt( to_string(ii),sourcetdbs[ii]);
    arr = domdata->appendArray( "substanceSymbols");
    for(uint ii=0; ii<substanceSymbols.size(); ii++)
        arr->appendString( to_string(ii), substanceSymbols[ii]);
    // make query
    DBQueryData query( AQLreq, DBQueryData::qAQL );
    query.setBindVars( domdata.get() );
    query.setQueryFields( makeQueryFields() );

    ValuesTable reactQueryMatr = getDB()->loadRecords(query, getDataNames());

    // delete not unique
    if( unique )
    {
        ValuesTable reactMatr;
        for (const auto& subitem : reactQueryMatr)
        {
           auto symbol = subitem[getDataName_DataIndex()["symbol"]];
           if ( reactMatr.empty() ||
                reactMatr.back()[getDataName_DataIndex()["symbol"]] != symbol )
                       reactMatr.push_back(subitem);
        }
       reactQueryMatr = move(reactMatr);
    }

    vector<string> reactSymbols;
    for (const auto& subitem : reactQueryMatr)
      reactSymbols.push_back(subitem[getDataName_DataIndex()["symbol"]]);

    setDefaultLevelForReactionDefinedSubst(reactQueryMatr);
    pimpl->valuesTable =          move(reactQueryMatr);
    return                reactSymbols;
}




}
