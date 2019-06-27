#ifndef PARSEBSONTRAVERSALDATA
#define PARSEBSONTRAVERSALDATA

#include <string>
#include <map>
#include <vector>
#include <set>
#include <jsonio/jsondom.h>

namespace ThermoFun {

struct DatabaseClient;
struct Database;
struct Substance;
struct Reaction;
struct Element;
struct ThermoParametersSubstance;
struct ThermoPropertiesSubstance;
struct ThermoParametersReaction;
struct ThermoPropertiesReaction;
struct Method;

using List_VertexId_VertexType    = std::vector<std::pair<std::string, std::string>>;

enum DefinesSubstLevelMode {
    followAll= 0,
    followGiven,
    followDiefferentPerSubstance
};

struct DefinesSubstLevelOptions {
    /// map of substance symbol and level for the edge defines connected to the substance
    std::map<std::string, std::string> substSymbol_definesLevel;

    /// current level for the traversal operation
    std::string level = "0";

    /// mode of using levels for traversal (all: collect all connected; single: collect all connected using one
    /// defines level for all substances; multiple: collect all connectec using different levels for different
    /// substances (from map substSymbol_definesLevel)
    DefinesSubstLevelMode definesSubstLevelMode = DefinesSubstLevelMode::followAll;
};

//DefinesSubstLevelOptions levelOptionsDefault;
/// Parses an element bson object and loads its data into the element structure
/// @param bso bson object
auto parseElement (const std::string& data) -> Element;

/// Parses a substance bson object and loads its data into the substance structure
/// @param bso bson object
auto parseSubstance (const std::string& data) -> Substance;

/// Parses a reaction bson object and loads its data into the reaction structure
/// @param bso bson object
auto parseReaction (const std::string& data) -> Reaction;

/// Parses an element bson object and loads its data into the element structure
/// @param bso bson object
auto parseElement (const jsonio::JsonDom *object) -> Element;

/// Parses a substance bson object and loads its data into the substance structure
/// @param bso bson object
auto parseSubstance (const jsonio::JsonDom *object) -> Substance;

/// Parses a bson object and loads the parameters into the internal sturcture
/// @param bso bson object
auto thermoParamSubst (const jsonio::JsonDom *object, std::string name) -> ThermoParametersSubstance;
auto thermoParamSubst (const jsonio::JsonDom *object, std::string name, std::string prop_name, ThermoParametersSubstance& ps) -> void;

/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefPropSubst (const jsonio::JsonDom *object, std::string name) -> ThermoPropertiesSubstance;

/// Parses a reaction bson object and loads its data into the reaction structure
/// @param bso bson object
auto parseReaction (const jsonio::JsonDom *object) -> Reaction;

/// Parses a bson object and loads the parameters into the internal sturcture
/// @param bso bson object
auto thermoParamReac (const jsonio::JsonDom *object, std::string name, ThermoParametersReaction& pr) -> void;

/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefPropReac (const jsonio::JsonDom *object, std::string name) -> ThermoPropertiesReaction;

auto databaseFromRecordList(const DatabaseClient &dbc, const List_VertexId_VertexType &recordList, DefinesSubstLevelOptions levelOptions = DefinesSubstLevelOptions()) -> Database;

auto getTPMethods(const std::vector<std::string>& vkbuf) -> std::vector<std::pair<std::string, Method>>;
auto getTPMethods(const jsonio::JsonDom *object, const std::vector<std::string>& vkbuf, Substance& s) -> void;
auto getTPMethods(const std::vector<std::string> &vkbuf, Reaction &r) -> void;

}


#endif // READFILES

