#include "StandardEntropyCpIntegration.h"
#include "ThermoProperties.h"
#include "Substance.h"

namespace ThermoFun {

// when S and Cp(const) are given
auto thermoPropertiesEntropyCpIntegration(real TK, real /*Pbar*/, Substance substance) -> ThermoPropertiesSubstanceAD
{
    ThermoPropertiesSubstanceAD thermo_properties_PT;
    ThermoPropertiesSubstance   thermo_properties_PrTr = substance.thermoReferenceProperties();

    real TrK = substance.referenceT()/* + C_to_K*/;

    // the reference properties are constants
    real S  = Reaktoro_::constant(thermo_properties_PrTr.entropy);
    real G  = Reaktoro_::constant(thermo_properties_PrTr.gibbs_energy);
    real H  = Reaktoro_::constant(thermo_properties_PrTr.enthalpy);
    real Cp = Reaktoro_::constant(thermo_properties_PrTr.heat_capacity_cp);

    if (thermo_properties_PrTr.entropy.sta.first != Reaktoro_::Status::notdefined)
        G -= S * (TK - TrK);
    if (thermo_properties_PrTr.heat_capacity_cp.sta.first != Reaktoro_::Status::notdefined)
    {
        S += (Cp * log(TK/TrK) );
        G -= (Cp * ( TK * log(TK/TrK) - (TK - TrK) ) );
        H += (Cp * (TK - TrK) );
    }

    thermo_properties_PT.heat_capacity_cp   = Cp;
    thermo_properties_PT.gibbs_energy       = G;
    thermo_properties_PT.enthalpy           = H;
    thermo_properties_PT.entropy            = S;

    return thermo_properties_PT;
}
} // namespace ThermoFun
