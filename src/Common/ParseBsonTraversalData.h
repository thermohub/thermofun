#ifndef PARSEBSONTRAVERSALDATA
#define PARSEBSONTRAVERSALDATA

#include <string>
#include <map>
#include <vector>
#include <set>

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
auto parseElement (const char *data) -> Element;

/// Parses a substance bson object and loads its data into the substance structure
/// @param bso bson object
auto parseSubstance (const char *data) -> Substance;

/// Parses a bson object and loads the parameters into the internal sturcture
/// @param bso bson object
auto thermoParamSubst (const char *data, std::string name) -> ThermoParametersSubstance;

/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefPropSubst (const char *data, std::string name) -> ThermoPropertiesSubstance;

/// Parses a reaction bson object and loads its data into the reaction structure
/// @param bso bson object
auto parseReaction (const char *data) -> Reaction;

/// Parses a bson object and loads the parameters into the internal sturcture
/// @param bso bson object
auto thermoParamReac (const char *data, std::string name) -> ThermoParametersReaction;

/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefPropReac (const char *data, std::string name) -> ThermoPropertiesReaction;

auto databaseFromRecordList(const DatabaseClient &dbc, const List_VertexId_VertexType &recordList, DefinesSubstLevelOptions levelOptions = DefinesSubstLevelOptions()) -> Database;

}


#endif // READFILES

