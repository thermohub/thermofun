#ifndef RYZHENKOBRYZGALYN
#define RYZHENKOBRYZGALYN

#include "Common/ThermoScalar.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace TCorrPT {

auto thermoPropertiesRyzhenkoBryzgalin(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, PropertiesSolvent wp) -> ThermoPropertiesReaction;

}

#endif // RYZHENKOBRYZGALYN

