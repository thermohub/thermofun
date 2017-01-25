#ifndef READFILES
#define READFILES

#include <string>
//#include "Database.h"

// bsonio includes
#include "bsonio/json2cfg.h"
#include "bsonio/thrift_node.h"
//#include "bsonio/impex_types.h"

//#include "bsonio/dbbase.h"
//#include "bsonio/nejdb.h"

namespace ThermoFun {

struct Substance;
struct ThermoParametersSubstance;
struct ThermoPropertiesSubstance;
struct Reaction;
struct ThermoParametersReaction;
struct ThermoPropertiesReaction;

/// Parses a substance bson object and loads its data into the substance structure
/// @param bso bson object
auto parseSubstance (const char *data) -> Substance;

/// Parses a bson object and loads the parameters into the internal sturcture
/// @param bso bson object
auto thermoParamSubst (const char *data, string name) -> ThermoParametersSubstance;

/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefPropSubst (const char * data, string name) -> ThermoPropertiesSubstance;

/// Parses a reaction bson object and loads its data into the reaction structure
/// @param bso bson object
auto parseReaction (const char *data) -> Reaction;

/// Parses a bson object and loads the parameters into the internal sturcture
/// @param bso bson object
auto thermoParamReac (const char *data, string name) -> ThermoParametersReaction;

/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefPropReac (const char * data, string name) -> ThermoPropertiesReaction;

}


#endif // READFILES

