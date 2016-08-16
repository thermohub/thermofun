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

namespace TCorrPT {

struct Substance;
struct ThermoParametersSubstance;
struct ThermoPropertiesSubstance;

/// Parses a substance bson object and loads its data into the substance structure
/// @param bso bson object
auto parseSubstance (const char *data) -> Substance;

/// Parses a bson object and loads the parameters into the internal sturcture
/// @param bso bson object
auto thermoParam (const char *data) -> ThermoParametersSubstance;

/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefProp (const char * data) -> ThermoPropertiesSubstance;
}


#endif // READFILES

