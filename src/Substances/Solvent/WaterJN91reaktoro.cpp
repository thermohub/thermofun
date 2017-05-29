#include "Substances/Solvent/WaterJN91reaktoro.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto electroPropertiesWaterJNreaktoro(const WaterElectroState& wts) -> ElectroPropertiesSolvent
{
    ElectroPropertiesSolvent eps;

    eps.bornN = wts.bornN;
    eps.bornQ = wts.bornQ;
    eps.bornU = wts.bornU;
    eps.bornX = wts.bornX;
    eps.bornY = wts.bornY;
    eps.bornZ = wts.bornZ;

    eps.epsilon   = wts.epsilon;
    eps.epsilonP  = wts.epsilonP;
    eps.epsilonPP = wts.epsilonPP;
    eps.epsilonT  = wts.epsilonT;
    eps.epsilonTP = wts.epsilonTP;
    eps.epsilonTT = wts.epsilonTT;

    return eps;
}

}
