#include "Substances/Solute/SoluteHKFreaktoro.h"
#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoParameters.h"

namespace ThermoFun {

/// The reference temperature assumed in the HKF equations of state (in units of K)
const double referenceTemperature = 298.15;

/// The reference temperature assumed in the HKF equations of state (in units of bar)
const double referencePressure = 1.0;

/// The reference dielectric constant of water \epsilon
const double referenceDielectricConstant = 78.24385513;

/// The reference Born function Z (dimensionless)
const double referenceBornZ = -1.278055636e-02;

/// The reference Born function Y (dimensionless)
const double referenceBornY = -5.795424563e-05;

/// The reference Born function Q (dimensionless)
const double referenceBornQ =  6.638388994e-12;

/// The reference Born function N (dimensionless)
const double referenceBornN = -2.321814455e-20;

/// The reference Born function U (dimensionless)
const double referenceBornU = 4.872982291e-14;

/// The reference Born function X (dimensionless)
const double referenceBornX = -3.060388224e-07;

/// The \eta constant in the HKF model (in units of (A*cal)/mol)
const double eta = 1.66027e+05;

/// The constant characteristics \Theta of the solvent (in units of K)
const double theta = 228;

/// The constant characteristics \Psi of the solvent (in units of bar)
const double psi = 2600;

auto thermoPropertiesAqSoluteHKFreaktoro(Reaktoro_::Temperature TK, Reaktoro_::Pressure P, Substance subst, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolvent& wes, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance
{
    // Get the HKF thermodynamic data of the species
    auto hkf = subst.thermoParameters().HKF_parameters;
    auto refProp = subst.thermoReferenceProperties();

    if (hkf.size() == 0)
    {
        Exception exception;
        exception.error << "Error in HKFrektoro EOS";
        exception.reason << "The HKF paramteres for "<< subst.symbol() << " are not defined or are not correclty initialized.";
        exception.line = __LINE__;
        RaiseError(exception);
    }
    if (hkf[0] == 0.0 || hkf[1] == 0.0 || hkf[2] == 0.0)
    {
        Exception exception;
        exception.error << "Error in HKFrektoro EOS";
        exception.reason << "The HKF paramteres for "<< subst.symbol() << " are not defined or are not correclty initialized.";
        exception.line = __LINE__;
        RaiseError(exception);
    }

    // Auxiliary variables
    const auto Pbar = P * 1.0e-05;
    const auto Tr   = referenceTemperature;
    const auto Pr   = referencePressure;
    const auto Zr   = referenceBornZ;
    const auto Yr   = referenceBornY;
    const auto Gf   = refProp.gibbs_energy / cal_to_J;
    const auto Hf   = refProp.enthalpy / cal_to_J;
    const auto Sr   = refProp.entropy / cal_to_J;
    const auto a1   = hkf[0];
    const auto a2   = hkf[1];
    const auto a3   = hkf[2];
    const auto a4   = hkf[3];
    const auto c1   = hkf[4];
    const auto c2   = hkf[5];
    const auto wr   = hkf[6];
    const auto w    = aes.w;
    const auto wT   = aes.wT;
    const auto wP   = aes.wP*1e05; // from 1/Pa to 1/bar
    const auto wTT  = aes.wTT;
    const auto Z    = wes.bornZ;
    const auto Y    = wes.bornY;
    const auto Q    = wes.bornQ*1e05; // from 1/Pa to 1/bar
    const auto X    = wes.bornX;

    // Calculate the standard molal thermodynamic properties of the aqueous species
    auto V = 0.4184004e2 * (a1 + a2/(psi + Pbar) +
        (a3 + a4/(psi + Pbar))/(TK - theta) - w*Q - (Z + 1)*wP);

    auto G = Gf - Sr*(TK - Tr) - c1*(TK*log(TK/Tr) - TK + Tr)
        + a1*(Pbar - Pr) + a2*log((psi + Pbar)/(psi + Pr))
        - c2*((1.0/(TK - theta) - 1.0/(Tr - theta))*(theta - TK)/theta
        - TK/(theta*theta)*log(Tr/TK * (TK - theta)/(Tr - theta)))
        + 1.0/(TK - theta)*(a3*(Pbar - Pr) + a4*log((psi + Pbar)/(psi + Pr)))
        - w*(Z + 1) + wr*(Zr + 1) + wr*Yr*(TK - Tr);

    auto H = Hf + c1*(TK - Tr) - c2*(1.0/(TK - theta) - 1.0/(Tr - theta))
        + a1*(Pbar - Pr) + a2*log((psi + Pbar)/(psi + Pr))
        + (2*TK - theta)/pow(TK - theta, 2)*(a3*(Pbar - Pr)
        + a4*log((psi + Pbar)/(psi + Pr)))
        - w*(Z + 1) + w*TK*Y + TK*(Z + 1)*wT + wr*(Zr + 1) - wr*Tr*Yr;

    auto S = Sr + c1*log(TK/Tr) - c2/theta*(1.0/(TK - theta)
        - 1.0/(Tr - theta) + log(Tr/TK * (TK - theta)/(Tr - theta))/theta)
        + 1.0/pow(TK - theta, 2)*(a3*(Pbar - Pr) + a4*log((psi + Pbar)/(psi + Pr)))
        + w*Y + (Z + 1)*wT - wr*Yr;

    auto Cp = c1 + c2/pow(TK - theta, 2) - (2*TK/pow(TK - theta, 3))*(a3*(Pbar - Pr)
        + a4*log((psi + Pbar)/(psi + Pr))) + w*TK*X + 2*TK*Y*wT + TK*(Z + 1)*wTT;

    auto U = H - Pbar*V;

    auto A = U - TK*S;

    // Convert the thermodynamic properties of the gas to the standard units
    V  *= 1e-01/**cal_to_J/bar_to_Pa*/;
    G  *= cal_to_J;
    H  *= cal_to_J;
    S  *= cal_to_J;
    U  *= cal_to_J;
    A  *= cal_to_J;
    Cp *= cal_to_J;

    ThermoPropertiesSubstance tps;
    tps.volume           = V;
    tps.gibbs_energy     = G;
    tps.enthalpy         = H;
    tps.entropy          = S;
    tps.internal_energy  = U;
    tps.helmholtz_energy = A;
    tps.heat_capacity_cp = Cp;
    tps.heat_capacity_cv = tps.heat_capacity_cp; // approximate Cp = Cv for an aqueous solution

    subst.checkCalcMethodBounds("HKF model", TK.val-C_to_K, Pbar.val, tps);
    if (wp.density >= 1400 || wp.density<=600)
    {
        setMessage(Reaktoro_::Status::calculated, "HKF model: outside of 600-1400 kg/m3 density of pure H2O interval", tps);
    }

    return tps;
}


auto speciesElectroStateHKF(const FunctionG& g, Substance species) -> ElectroPropertiesSubstance
{
    // Get the HKF thermodynamic parameters of the aqueous species
    auto hkf = species.thermoParameters().HKF_parameters;

    // The species electro instance to be calculated
    ElectroPropertiesSubstance se;

    // Check if the aqueous species is neutral or H+, and set its electrostatic data accordingly
    if(species.charge() == 0.0 || (species.symbol() == "H+"))
    {
        se.w   = hkf[6] /*hkf.wref*/;
        se.wT  = 0.0;
        se.wP  = 0.0;
        se.wTT = 0.0;
        se.wTP = 0.0;
        se.wPP = 0.0;
    }
    else
    {
        const auto z = species.charge();
        const auto wref = hkf[6] /*hkf.wref*/;

        const auto reref = z*z/(wref/eta + z/3.082);
        const auto re    = reref + std::abs(z) * g.g;

        const auto X1 =  -eta * (std::abs(z*z*z)/(re*re) - z/pow(3.082 + g.g, 2));
        const auto X2 = 2*eta * (z*z*z*z/(re*re*re) - z/pow(3.082 + g.g, 3));

        se.re    = re;
        se.reref = reref;
        se.w     = eta * (z*z/re - z/(3.082 + g.g));
        se.wT    = X1 * g.gT;
        se.wP    = X1 * g.gP;
        se.wTT   = X1 * g.gTT + X2 * g.gT * g.gT;
        se.wTP   = X1 * g.gTP + X2 * g.gT * g.gP;
        se.wPP   = X1 * g.gPP + X2 * g.gP * g.gP;
    }

    return se;
}


auto functionG(Reaktoro_::Temperature T, Reaktoro_::Pressure P, const PropertiesSolvent& ps) -> FunctionG
{
    // The function G
    FunctionG funcG;

    // The temperature in units of celsius and pressure in units of bar
    const auto TdegC = T - 273.15;
    const auto Pbar  = P * 1.0e-5;

    if (ps.density >= 1400)
    {
        Exception exception;
        exception.error << "Error in functionG";
        exception.reason << "water density higher than 1.4 g*cm-3, Dw = "
            << ps.density / 1000 << "g*cm-3.";
        exception.line = __LINE__;
        RaiseError(exception);
    }

    // Check if the point (T,P) is inside region III or the shaded region in Fig. 6 of
    // Shock and others (1992), on page 809. In this case, we assume the g function to be zero.
    if(ps.density > 1000.0 || ps.density < 350.0)
        return funcG;

    // Auxiliary references
    auto& g   = funcG.g;
    auto& gT  = funcG.gT;
    auto& gP  = funcG.gP;
    auto& gTT = funcG.gTT;
    auto& gTP = funcG.gTP;
    auto& gPP = funcG.gPP;

    // Use equations (24)-(31) of Shock and others (1992) to compute `g` and its derivatives on region I
    const auto ag1 = -2.037662;
    const auto ag2 =  5.747000e-03;
    const auto ag3 = -6.557892e-06;

    const auto bg1 =  6.107361;
    const auto bg2 = -1.074377e-02;
    const auto bg3 =  1.268348e-05;

    const auto ag = ag1 + ag2*TdegC + ag3*TdegC*TdegC;
    const auto bg = bg1 + bg2*TdegC + bg3*TdegC*TdegC;

    const auto agT = ag2 + 2*ag3*TdegC;
    const auto bgT = bg2 + 2*bg3*TdegC;

    const auto agTT = 2*ag3;
    const auto bgTT = 2*bg3;

    const auto r =  ps.density/1000.0;

    const auto alpha  = -ps.densityT/ps.density;
    const auto beta   =  ps.densityP/ps.density;
    const auto alphaT = -ps.densityTT/ps.density + alpha*alpha;
    const auto alphaP = -ps.densityTP/ps.density - alpha*beta;
    const auto betaP  =  ps.densityPP/ps.density - beta*beta;

    g   =  ag * pow(1 - r, bg);
    gT  =   g * (agT/ag + bgT*log(1 - r) + r*alpha*bg/(1 - r));
    gP  =  -g * r*beta*bg/(1 - r);
    gTT =   g * (agTT/ag - pow(agT/ag, 2) + bgTT*log(1 - r) + r*alpha*bg/(1 - r) * (2*bgT/bg + alphaT/alpha - alpha - r*alpha/(1 - r))) + gT*gT/g;
    gTP =  gP * (bgT/bg - alpha - alphaP/beta - r*alpha/(1 - r)) + gP*gT/g;
    gPP =  gP * (gP/g + beta + betaP/beta + r*beta/(1 - r));

    // Check if the point (T,P) is inside region II, as depicted in Fig. 6 of Shock and others (1992), on page 809
    if(TdegC > 155.0 && TdegC < 355.0 && Pbar < 1000.0)
    {
        // Use equations (32)-(44) of Shock and others (1992) to compute the function g and its partial derivatives on region II
        const auto af1 =  3.666660e+01; // unit: K
        const auto af2 = -1.504956e-10; // unit: A:bar^{-3}
        const auto af3 =  5.017990e-14; // unit: A:bar^{-4}

        const auto auxT  = (TdegC - 155)/300;
        const auto auxT1 = pow(auxT, 4.8);
        const auto auxT2 = pow(auxT, 16);

        const auto auxP  = 1000 - Pbar;
        const auto auxP1 = pow(auxP, 3);
        const auto auxP2 = pow(auxP, 4);

        const auto ft   = auxT1 + af1*auxT2;
        const auto ftT  = ( 4.80 * auxT1 +  16.0 * af1*auxT2)/(300 * auxT);
        const auto ftTT = (18.24 * auxT1 + 240.0 * af1*auxT2)/pow(300 * auxT, 2);

        const auto fp   =  af2*auxP1 + af3*auxP2;
        const auto fpP  = -(3.0 * af2*auxP1 +  4.0 * af3*auxP2)/(auxP*1.e+5);       // convert derivative from bar to Pa
        const auto fpPP =  (6.0 * af2*auxP1 + 12.0 * af3*auxP2)/pow(auxP*1.e+5, 2); // convert derivative from bar^2 to Pa^2

        g   -= ft  * fp;
        gT  -= fp  * ftT;
        gP  -= ft  * fpP;
        gTT -= fp  * ftTT;
        gTP -= ftT * fpP;
        gPP -= ft  * fpPP;
    }

    return funcG;
}

}
