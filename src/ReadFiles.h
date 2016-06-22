#ifndef READFILES
#define READFILES

#include <string>
//#include "Database.h"

// bsonio includes
#include "bsonio/json2cfg.h"
#include "bsonio/thrift_node.h"
#include "bsonio/impex_types.h"

namespace TCorrPT {

struct Substance;
struct ThermoParametersSubstance;
struct ThermoPropertiesSubstance;

auto parseSubstance (bson bso) -> Substance;

auto thermoParam(bson bso) -> ThermoParametersSubstance;

auto thermoRefProp (bson bso) -> ThermoPropertiesSubstance;
}


#endif // READFILES

