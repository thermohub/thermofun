#include "TraversalData.h"
#include "AQLqueries.h"

// ThermoFun includes
#include "jsonio/jsondomfree.h"
#include "jsonio/jsondomschema.h"
#include "SubstanceData.h"
#include "ReactionData.h"
#include "Database.h"
#include "Substance.h"
#include "Reaction.h"
//#include "Common/ParseBsonTraversalData.h"
#include "Common/Exception.h"
#include "OptimizationUtils.h"
#include "Common/ParseJsonToData.h"

#include <algorithm>

#include <chrono>  // for high_resolution_clock
#include "nlohmann/json.hpp"

using namespace jsonio;
using json = nlohmann::json;

namespace ThermoFun
{

//const std::vector<std::string> queryFieldsVertex       = {"_id", "_label", "_type", "properties.symbol"};
//const std::vector<std::string> queryFieldsEdgeDefines  = { "_from", "_label"};
//const std::vector<std::string> queryFieldsEdgeTakes    = { "_from", "_label", "properties.stoi_coeff" };

using GetlinkedDataFromId  = std::function<VertexId_VertexType(std::string)>;
using GetlinkedDataFromIds  = std::function<VertexId_VertexType(std::vector<std::string>)>;

template<typename T>
bool isEqual(std::vector<T> const &v1, std::vector<T> const &v2)
{
    return (v1.size() == v2.size() &&
            std::equal(v1.begin(), v1.end(), v2.begin()));
}

enum DefinesLevelMode {
    all= 0,
    single,
    multiple
};

struct TraversalData::Impl
{
    /// map of substance symbol and level for the edge defines connected to the substance
    std::map<std::string, std::string> substSymbol_definesLevel;

    /// current level for the traversal operation
    std::string level = "0";

    /// mode of using levels for traversal (all: collect all connected; single: collect all connected using one
    /// defines level for all substances; multiple: collect all connectec using different levels for different
    /// substances (from map substSymbol_definesLevel)
    DefinesLevelMode definesLevelMode = DefinesLevelMode::single;

    /// map of database ids and bson record of vertexes
//    std::map<std::string, bson> idBsonRec;

    /// pointer to substance data for access to substance records
    SubstanceData_  *substData;

    /// pointer to reaction data for access to reaction records
    ReactionData_   *reactData;

    GetlinkedDataFromId  get_linked_data_from_id_fn;
    GetlinkedDataFromIds  get_linked_data_from_ids_fn;

    Impl(SubstanceData_ *asubstData,  ReactionData_ *areactData ):
     substData( asubstData ), reactData( areactData )
    {
        get_linked_data_from_id_fn = [=](string idRecord) {
            return getlinkedDataFromId(idRecord);
        };

        get_linked_data_from_id_fn = memoize(get_linked_data_from_id_fn);

        get_linked_data_from_ids_fn = [=](vector<string> idList) {
            return getLinkedDataFromIds(idList);
        };

        get_linked_data_from_ids_fn = memoize(get_linked_data_from_ids_fn);
    }

    auto getDefinesLevel (std::string jsonrecord) -> std::string
    {
        std::string level_;

        switch(definesLevelMode)
        {
        case DefinesLevelMode::all         : level_ = "-1";  // follows all connected data
            break;
        case DefinesLevelMode::single      : level_ = level;; // follows edges defines with level
            break;
        case DefinesLevelMode::multiple    : {
            std::string substSymb;
//            std::string key = idSubst;

//            string jsonrecord = substData->getJsonRecordVertex(key);
            auto domdata = jsonio::unpackJson( jsonrecord );
            domdata->findValue("properties.symbol", substSymb);

            //record = pimpl->substData->getJsonBsonRecordVertex(key).second;
            //bsonio::bson_to_key( record.data, "properties.symbol", substSymb );
            if (substSymbol_definesLevel.find(substSymb) != substSymbol_definesLevel.end()) // follows edges defines with specific leveles for substSymbols
                level_ = substSymbol_definesLevel[substSymb];   // if the substance symbol is not found in the map, it uses the default level
            else
                level_ = level;
        }
            break;
        }

        return level_;
    }

    auto getlinkedDataFromId(std::string id_) -> VertexId_VertexType
    {
        string _idRecord, _label, /*_type, _symbol,*/ level_;
        VertexId_VertexType result;

        string jsonrecord =  substData->getJsonRecordVertex(id_);
        auto domdata = jsonio::unpackJson( jsonrecord );
        domdata->findValue("_id",    _idRecord);
        domdata->findValue("_label", _label);

        // if not in the database proceed
        if (!result.count(_idRecord))
        {
            if (_label == "substance")
            {
                // get level mode
                level_ = getDefinesLevel(jsonrecord);
                result.insert(std::pair<std::string,std::string>(_idRecord, "substance"));
                followIncomingEdgeDefines(_idRecord, result, level_);
            }
            if (_label == "reaction")
            {
                result.insert(std::pair<std::string,std::string>(_idRecord, "reaction"));
                followIncomingEdgeTakes(_idRecord, result);
            }
        }
        return result;
    }

    void followIncomingEdgeDefines(std::string _idSubst, VertexId_VertexType &result, string level_)
    {
        string _idReact = "";
        string _resultDataReac;
        vector<string> _resultDataEdge = substData->queryInEdgesDefines(_idSubst, level_ );

        //std::cout << "incoming defines: "<< _idSubst << endl;

        for(std::size_t i = 0; i < _resultDataEdge.size(); i++)
        {
            _idReact = _resultDataEdge[i];
            _resultDataReac = reactData->getJsonRecordVertex(_idReact);

            // if not in the database
            if (!result.count(_idReact))
            {
                result.insert((std::pair<std::string,std::string>(_idReact, "reaction")));
                followIncomingEdgeTakes(_idReact, result);
            }
        }
    }

    void followIncomingEdgeTakes(std::string _idReact, VertexId_VertexType &result)
    {
        string _idSubst = ""; string level_ = "0";
        string _resultDataSubst;
        vector<string> _resultDataEdge  = reactData->queryInEdgesTakes(_idReact );

        //std::cout << "incoming takes: "<< _idReact << endl;

        for(std::size_t i = 0; i < _resultDataEdge.size(); i++)
        {
            _idSubst = extractStringField("_from", _resultDataEdge[i]);
            _resultDataSubst = substData->getJsonRecordVertex(_idSubst);

            // if not in the database
            if (!result.count(_idSubst))
            {
                level_ = getDefinesLevel(_resultDataSubst);
                result.insert((std::pair<std::string,std::string>(_idSubst, "substance")));
                followIncomingEdgeDefines(_idSubst, result, level_);
            }
        }
    }

    auto queryIdLabel(vector<string> idList, string AQLreq, string bindValues) -> ValuesTable
    {
        shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
        auto arr = domdata->appendArray(bindValues);

        jsonio::JsonDom *object;

        for(std::size_t ii=0; ii<idList.size(); ii++)
        {
            arr->appendValue(std::to_string(ii), idList[ii]);
        }

        // make query
        DBQueryData query( AQLreq, DBQueryData::qAQL );
        query.setBindVars( domdata.get() );
        jsonio::QueryFields qfields;
        qfields["_id"] = "_id";
        qfields["_label"] = "_label";
        query.setQueryFields(qfields);
        return substData->getDB()->downloadDocuments(query, {"_id", "_label"});
    }
    
    auto querySubstancesReactionIds(vector<string> idList, string AQLreq, string bindValues) -> ValuesTable
    {
        shared_ptr<JsonDomFree> domdata(JsonDomFree::newObject());
        auto arr = domdata->appendArray(bindValues);

        jsonio::JsonDom *object;

        for(std::size_t ii=0; ii<idList.size(); ii++)
        {
            arr->appendValue(std::to_string(ii), idList[ii]);
        }

        // make query
        DBQueryData query( AQLreq, DBQueryData::qAQL );
        query.setBindVars( domdata.get() );
        jsonio::QueryFields qfields;
        qfields["substances"] = "substances";
        qfields["reactions"] = "reactions";
        qfields["ids"]="ids";
        query.setQueryFields(qfields);
        return substData->getDB()->downloadDocuments(query, {"substances", "reactions", "ids"});
    }

    auto getLinkedDefines(vector<string> idList) -> ValuesTable
    {
        string AQLreq = "FOR r IN @records \n"
                        "   FOR v IN 1..1 INBOUND r defines \n"
                        "       RETURN {_id : v._id, _label : v._label}";

        return queryIdLabel(idList, AQLreq, "records");
    }

    auto getLinkedTakes(vector<string> idList) -> ValuesTable
    {
        string AQLreq = "FOR r IN @records \n"
                        "   FOR v IN 1..1 INBOUND r takes \n"
                        "       RETURN {_id : v._id, _label : v._label}";

        return queryIdLabel(idList, AQLreq, "records");
    }

    auto getIdListData(vector<string> idList) -> ValuesTable
    {
        string AQLreq = "FOR r IN substances OR reactions \n"
                        "   FILTER r._id IN @records \n"
                        "       RETURN {_id : r._id, _label : r._label}";

        return queryIdLabel(idList, AQLreq, "records");
    }

    auto getLinkedDataFromIds(vector<string> idList) -> VertexId_VertexType
    {
        VertexId_VertexType result;
        ValuesTable queryMatr;
        std::vector<string>::iterator it;
        auto s  = idList.size();

        queryMatr = getIdListData(idList);
        for (auto q : queryMatr)
        {
            result[q[0]] = q[1];
        }

        do {
            s  = idList.size();
            queryMatr = getLinkedDefines(idList);

            for (auto q : queryMatr)
            {
                result[q[0]] = q[1];
                it = std::find (idList.begin(), idList.end(), q[0]);
                if (it == idList.end())
                    idList.push_back(q[0]);
            }

            queryMatr = getLinkedTakes(idList);

            for (auto q : queryMatr)
            {
                result[q[0]] = q[1];
                it = std::find (idList.begin(), idList.end(), q[0]);
                if (it == idList.end())
                    idList.push_back(q[0]);
            }

        } while( idList.size() != s );

        return result;
    }
    
    auto getConnectedData(vector<string> idList) -> ValuesTable
    {
        ValuesTable queryMatr;
        auto s  = idList.size();
        
        std::vector<string>::iterator itl;

        do {
              s  = idList.size();
              queryMatr = querySubstancesReactionIds(idList, aql_fun_substances_reactions_ids, "ids");
              json jarr = json::parse(queryMatr[0][2]);

              if (s != jarr.size())
                  for (auto it = jarr.begin(); it != jarr.end(); ++it)
                  {
                      itl = std::find (idList.begin(), idList.end(), it.value().get<string>());
                      if (itl == idList.end())
                          idList.push_back(it.value().get<string>());
                  }
        } while( idList.size() != s );
        
        return queryMatr;
    }

    auto getResult(vector<string> idList, vector<std::size_t> selNdx ={}) -> VertexId_VertexType
    {
        VertexId_VertexType result, result2, r;
        // Record start time
        auto start = std::chrono::high_resolution_clock::now();

//        if (selNdx.size() == 0)
//        {
//            for (std::size_t ii=0; ii<idList.size(); ii++)
//            {
//                r = get_linked_data_from_id_fn(idList[ii]);
//                result.insert(r.begin(), r.end());
//            }
//        } else
//        {
//            for( size_t ii=0; ii<selNdx.size(); ii++ )
//            {
//                r = get_linked_data_from_id_fn(idList[selNdx[ii]]);
//                result.insert(r.begin(), r.end());
//            }
//        }

        if (idList.size()>0)
            result = get_linked_data_from_ids_fn(idList);

        // Record end time
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;

        cout << "finished getresult, in: " << elapsed.count() << endl;

        auto start2 = std::chrono::high_resolution_clock::now();

        auto data = getConnectedData(idList);

        // Record end time
        auto finish2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed2 = finish2 - start2;
        cout << "finished getConnectedData, in: " << elapsed2.count() << endl;

        auto start3 = std::chrono::high_resolution_clock::now();

        json jSubstances = json::parse(data[0][0]);
        json jReactions = json::parse(data[0][1]);
        Database tdb;
        // The set of all aqueous species in the database
        SubstancesMap substances_map;
        // The set of all gaseous species in the database
        ReactionsMap reactions_map;

        for (auto it = jSubstances.begin(); it != jSubstances.end(); ++it)
        {
            Substance substance = parseSubstance(it.value().dump());
            json jSubst = it.value();
            if (jSubst.contains("reaction"))
            {
                substance.setReactionSymbol(jSubst["reaction"]);
                substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
            }
            if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
                substances_map[substance.symbol()] = substance;
            } else {
                errorSameSymbol("substance", substance.symbol(), __LINE__, __FILE__ );
            }
        }

        for (auto it = jReactions.begin(); it != jReactions.end(); ++it)
        {
            Reaction reaction = parseReaction(it.value().dump());

            if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
                reactions_map[reaction.symbol()] = reaction;
            } else {
                errorSameSymbol("reaction", reaction.symbol(), __LINE__, __FILE__ );
            }
        }

        // Record end time
        auto finish3 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed3 = finish3 - start3;
        cout << "finished getDatabase, in: " << elapsed3.count() << endl;

        return result;
    }

    auto getDatabase(const std::vector<std::string> idList) -> Database
    {
        auto start2 = std::chrono::high_resolution_clock::now();

        auto data = getConnectedData(idList);

        // Record end time
        auto finish2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed2 = finish2 - start2;
        cout << "finished getConnectedData, in: " << elapsed2.count() << endl;

        auto start3 = std::chrono::high_resolution_clock::now();

        json jSubstances = json::parse(data[0][0]); // [0] substances
        json jReactions = json::parse(data[0][1]); // [1] reactions; [2] list of ids
        Database tdb;
        // The set of all aqueous species in the database
        SubstancesMap substances_map;
        // The set of all gaseous species in the database
        ReactionsMap reactions_map;

        for (auto it = jSubstances.begin(); it != jSubstances.end(); ++it)
        {
            Substance substance = parseSubstance(it.value().dump());
            json jSubst = it.value();
            if (jSubst.contains("reaction"))
            {
                substance.setReactionSymbol(jSubst["reaction"]);
                substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
            }
            if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
                substances_map[substance.symbol()] = substance;
            } else {
                errorSameSymbol("substance", substance.symbol(), __LINE__, __FILE__ );
            }
        }

        for (auto it = jReactions.begin(); it != jReactions.end(); ++it)
        {
            Reaction reaction = parseReaction(it.value().dump());

            if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
                reactions_map[reaction.symbol()] = reaction;
            } else {
                errorSameSymbol("reaction", reaction.symbol(), __LINE__, __FILE__ );
            }
        }

        // Record end time
        auto finish3 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed3 = finish3 - start3;
        cout << "finished getDatabase, in: " << elapsed3.count() << endl;

        tdb.addMapReactions(reactions_map);
        tdb.addMapSubstances(substances_map);

        return tdb;
    }

    auto getDatabase(VertexId_VertexType resultTraversal) -> Database
    {
        string _idSubst, _idReact, substSymb; string level_ = level;
        Database tdb;
        auto result = resultTraversal;
        // Record start time
        auto start = std::chrono::high_resolution_clock::now();

        // The set of all aqueous species in the database
        SubstancesMap substances_map;
        // The set of all gaseous species in the database
        ReactionsMap reactions_map;

        // get substances and the reaction symbol if necessary
        for(auto iterator = result.begin(); iterator != result.end(); iterator++)
        {
            if (iterator->second == "substance")
            {
                _idSubst = iterator->first;

                string jsonrecord = substData->getJsonRecordVertex(_idSubst);
                auto domdata = jsonio::unpackJson( jsonrecord, "VertexSubstance" ); // with default values
                domdata->findValue("properties.symbol", substSymb);
                //record = pimpl->substData->getJsonBsonRecordVertex(_idSubst).second;
                //bsonio::bson_to_key( record.data, "properties.symbol", substSymb );

                level_ = getDefinesLevel(jsonrecord);
                Substance substance = parseSubstance(jsonrecord);

                // get reaction symbol which define substance with _idSubst
                string definesReactSymb = substData->definesReactionSymbol(_idSubst, level_);
                if (definesReactSymb != "")
                {
                    substance.setReactionSymbol(definesReactSymb);
                    substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
                }

                if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
                    substances_map[substance.symbol()] = substance;
                } else {
                    errorSameSymbol("substance", substance.symbol(), __LINE__, __FILE__ );
                }
            } else
                if (iterator->second == "reaction")
                {
                    _idReact = iterator->first;
                    string jsonrecord = reactData->getJsonRecordVertex(_idReact);
                    auto domdata = jsonio::unpackJson( jsonrecord, "VertexReaction" ); // with default values
                    //record = pimpl->reactData->getJsonBsonRecordVertex(_idReact).second;

                    Reaction reaction = ThermoFun::parseReaction(jsonrecord);

                    // get reactants by following reaction incoming takes edge
                    reaction.setReactants(reactData->reactantsCoeff(_idReact));

                    if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
                        reactions_map[reaction.symbol()] = reaction;
                    } else {
                        errorSameSymbol("reaction", reaction.symbol(), __LINE__, __FILE__ );
                    }
                }
        }
        tdb.addMapReactions(reactions_map);
        tdb.addMapSubstances(substances_map);

        // Record end time
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;

        cout << "finished database, in: " << elapsed.count() << endl;
        return tdb;
    }
};

TraversalData::TraversalData(SubstanceData_ *substData,  ReactionData_ *reactData) :
    pimpl(new Impl(substData, reactData ))
{ }

auto TraversalData::operator=(TraversalData other) -> TraversalData&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

//TraversalData::TraversalData(const TraversalData& other)
//: pimpl(new Impl(*other.pimpl))
//{}

TraversalData::~TraversalData()
{ }

// Public
auto TraversalData::getMapOfConnectedIds(vector<std::size_t> selNdx, vector<string> idsList, string level_) -> VertexId_VertexType
{
    pimpl->level = level_;
    pimpl->definesLevelMode = DefinesLevelMode::single;

    return pimpl->getResult(idsList, selNdx);
}

auto TraversalData::getMapOfConnectedIds( vector<string> idList, string level_) -> VertexId_VertexType
{
    pimpl->level = level_;
    pimpl->definesLevelMode = DefinesLevelMode::single;

    return pimpl->getResult(idList);
}

auto TraversalData::getMapOfConnectedIds(vector<string> idList, std::map<std::string, std::string> substSymbol_definesLevel) -> VertexId_VertexType
{
    pimpl->definesLevelMode = DefinesLevelMode::multiple;

    if (!substSymbol_definesLevel.empty())
    {
        pimpl->substSymbol_definesLevel = substSymbol_definesLevel;
    } else
    {
        pimpl->definesLevelMode = DefinesLevelMode::single;
    }
    return pimpl->getResult(idList);;
}

auto TraversalData::getDatabaseFromMapOfIds(VertexId_VertexType resultTraversal, string level_) -> Database
{
    pimpl->level = level_;
    pimpl->definesLevelMode = DefinesLevelMode::single;
    return pimpl->getDatabase(resultTraversal);
}

auto TraversalData::getDatabaseFromMapOfIds(VertexId_VertexType resultTraversal, std::map<std::string, std::string> substSymbol_definesLevel) -> Database
{
    if (!substSymbol_definesLevel.empty())
    {
        pimpl->definesLevelMode = DefinesLevelMode::multiple;
        pimpl->substSymbol_definesLevel = substSymbol_definesLevel;
    } else
    {
        pimpl->definesLevelMode = DefinesLevelMode::single;
    }
    return pimpl->getDatabase(resultTraversal);
}

VertexId_VertexType TraversalData::getMapOfConnectedIds( vector<string> idList )
{
    pimpl->definesLevelMode = DefinesLevelMode::all;
    return pimpl->getResult(idList);
}

auto TraversalData::linkedDataIds(const std::vector<std::string> aKeyList, std::vector<std::string> &substIds, std::vector<std::string> &reactIds) -> void
{
    reactIds.clear(); substIds.clear();
    for (auto idType : getMapOfConnectedIds(aKeyList))
    {
        if (idType.second == "reaction")
            reactIds.push_back((idType.first));
        if (idType.second == "substance")
            substIds.push_back((idType.first));
    }
}

auto TraversalData::getDatabase(const std::vector<std::string> &idList) -> Database
{
    return pimpl->getDatabase(idList);
}

// Private

}
