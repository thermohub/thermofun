#include "EmpiricalCpIntegration.h"
#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Substance.h"
#include <cmath>
//#include <iomanip>

namespace ThermoFun
{

auto thermoPropertiesEmpCpIntegration(const Reaktoro_::Pass& pass, Substance substance, bool* outsideBounds) -> ThermoPropertiesSubstanceAD
{
    real TK = pass.T;
    ThermoPropertiesSubstanceAD thermo_properties_PT;
    ThermoPropertiesSubstance thermo_properties_PrTr = substance.thermoReferenceProperties();
    SubstanceClass::type substance_class = substance.substanceClass();
    ThermoParametersSubstance thermo_parameters = substance.thermoParameters();

    real V;
    V = 0.0;
    int k = -1;
    std::vector<double> ac;
    auto TK_ = TK;
    for (unsigned i = 0; i < 16; i++)
    {
        ac.push_back(0.0);
    }

    auto TrK = substance.referenceT() /* + C_to_K*/;

    // the reference properties are constants
    real S = Reaktoro_::constant(thermo_properties_PrTr.entropy);
    real G = Reaktoro_::constant(thermo_properties_PrTr.gibbs_energy);
    real H = Reaktoro_::constant(thermo_properties_PrTr.enthalpy);

    if (thermo_parameters.Cp_coeff.size() == 0)
    {
        errorModelParameters("Cp empirical coefficients", substance.symbol() + " empirical Cp integration", __LINE__, __FILE__);
        return thermo_properties_PT;
    }

    if (thermo_parameters.temperature_intervals.size() == 0)
    {
        errorModelParameters("Cp temperature intervals", substance.symbol() + " empirical Cp integration", __LINE__, __FILE__);
        return thermo_properties_PT;
    }

    // A non-finite input temperature would compare false against every interval bound below
    // (NaN comparisons are always false), leaving k unresolved even after the out-of-bounds
    // fallback further down -- reject it up front instead of indexing with an unresolved k.
    if (!std::isfinite(TK_.val()))
    {
        errorModelParameters("Cp temperature intervals", substance.symbol() + " empirical Cp integration", __LINE__, __FILE__);
        return thermo_properties_PT;
    }

    // Cp_coeff[k] and Cp_coeff[j] (0 <= j <= k) are indexed by interval below, so every interval
    // needs a matching coefficient entry. Every interval also needs a lower and an upper bound,
    // with lower < upper, since both are indexed unconditionally further down.
    if (thermo_parameters.Cp_coeff.size() < thermo_parameters.temperature_intervals.size())
    {
        errorModelParameters("Cp empirical coefficients", substance.symbol() + " empirical Cp integration", __LINE__, __FILE__);
        return thermo_properties_PT;
    }

    // Intervals must be finite, non-empty (lower < upper), and non-overlapping/monotonic (each
    // interval's lower bound at or after the previous interval's upper bound) -- otherwise a
    // temperature in the overlap would be double-integrated by the j <= k loop further down.
    // Gaps between intervals are still allowed.
    for (size_t i = 0; i < thermo_parameters.temperature_intervals.size(); i++)
    {
        if (thermo_parameters.temperature_intervals[i].size() < 2 ||
            !std::isfinite(thermo_parameters.temperature_intervals[i][0]) ||
            !std::isfinite(thermo_parameters.temperature_intervals[i][1]) ||
            thermo_parameters.temperature_intervals[i][0] >= thermo_parameters.temperature_intervals[i][1] ||
            (i > 0 && thermo_parameters.temperature_intervals[i][0] < thermo_parameters.temperature_intervals[i - 1][1]))
        {
            errorModelParameters("Cp temperature intervals", substance.symbol() + " empirical Cp integration", __LINE__, __FILE__);
            return thermo_properties_PT;
        }
    }

    // get Cp interval
    for (size_t i = 0; i < thermo_parameters.temperature_intervals.size(); i++)
    {
        if ((thermo_parameters.temperature_intervals[i][0] <= TK) && (thermo_parameters.temperature_intervals[i][1] > TK))
        {
            k = static_cast<int>(i);
            break;
        }
    }

    bool k_outside_bounds = false;

    if (k < 0)
    {
        k_outside_bounds = true;

        if (pass.wrt != Reaktoro_::Wrt::P) // log once
        thfun_logger->warn(" {} {}: The given temperature: {} is not inside the specified interval/s for the Cp calculation.\n"
                           "The temperature is not inside the specified interval for the substance {}.",
                           __FILE__, __LINE__, TK_.val(), substance.symbol());

        if (TK_ <= thermo_parameters.temperature_intervals[0][0])
        {
            k = 0;
        }
        // ">=", not ">": the in-interval test above uses a strict "<" on the upper bound, so a
        // temperature exactly equal to the last interval's upper bound matches neither that test
        // nor a strict ">" here, leaving k unset. That left k == -1, which was then used a few
        // lines below to index Cp_coeff (and other vv<double> arrays) as an implicit size_t --
        // an out-of-bounds read that crashed (SIGSEGV) for any substance whose swept temperature
        // landed exactly on its Cp-interval upper bound (e.g. a single-interval substance ending
        // at 683.15 K, hit by a 10 K sweep step landing exactly there).
        else if (TK_ >= thermo_parameters.temperature_intervals[thermo_parameters.temperature_intervals.size() - 1][1])
        {
            k = static_cast<int>(thermo_parameters.temperature_intervals.size()) - 1;
        }
        else
        {
            // TK falls in a gap between two non-contiguous intervals (e.g. [0,100] and [200,300]
            // with TK = 101): pick whichever neighboring interval's bound is numerically closer,
            // instead of always clamping to the last interval regardless of which side TK is on.
            for (size_t i = 0; i + 1 < thermo_parameters.temperature_intervals.size(); i++)
            {
                if (TK_ >= thermo_parameters.temperature_intervals[i][1] &&
                    TK_ <= thermo_parameters.temperature_intervals[i + 1][0])
                {
                    double dist_lower = TK_.val() - thermo_parameters.temperature_intervals[i][1];
                    double dist_upper = thermo_parameters.temperature_intervals[i + 1][0] - TK_.val();
                    k = (dist_lower <= dist_upper) ? static_cast<int>(i) : static_cast<int>(i + 1);
                    break;
                }
            }
        }
    }

    //k = 0; fix

    // Defensive: with finite TK and validated, monotonic, non-overlapping intervals, the
    // resolution above always assigns k. Guard anyway before indexing Cp_coeff/temperature_intervals.
    if (k < 0)
    {
        errorModelParameters("Cp temperature intervals", substance.symbol() + " empirical Cp integration", __LINE__, __FILE__);
        return thermo_properties_PT;
    }

    for (unsigned i = 0; i < thermo_parameters.Cp_coeff[k].size(); i++)
    {
        if (i == 16)
            break;
        ac[i] = thermo_parameters.Cp_coeff[k][i];
    }

    auto Cp = (ac[0] +
                ac[1] * TK +
                ac[2] / (TK * TK) +
                ac[3] / (pow(TK, 0.5)) +
                ac[4] * (TK * TK) +
                ac[5] * (TK * TK * TK) +
                ac[6] * (TK * TK * TK * TK) +
                ac[7] / (TK * TK * TK) +
                ac[8] / TK +
                ac[9] * (pow(TK, (1 / 2))) /*+ ac[10]*log(T)*/);

    for (unsigned j = 0, ft = 0; j <= k; j++)
    {
        if (j == k)
            TK = TK_; // current T is the end T for phase transition Cp calculations
        else
            TK = thermo_parameters.temperature_intervals[j][1] /*+ C_to_K*/; // takes the upper bound from the j-th Tinterval (a constant)

        if (!j)
            TrK = substance.referenceT() /*+ C_to_K*/; // if j=0 the first interval should contain the reference T (Tcr)
        else
            TrK = thermo_parameters.temperature_intervals[j][0] /*+ C_to_K*/; // if j>0 then we are in a different Tinterval and the reference T becomes the lower bound of the interval

        auto Tst2 = TrK * TrK;
        auto Tst3 = Tst2 * TrK;
        auto Tst4 = Tst3 * TrK;
        auto Tst05 = std::sqrt(TrK);

        // going trough the phase transitions parameters in FtP
        //            for (unsigned ft = 0; ft < thermo_parameters.phase_transition_prop.size(); ft++)

        if (j && thermo_parameters.phase_transition_prop.size() == 0)
        {
            Exception exception;
            exception.error << "No phase transition properties present in the record.";
            exception.reason << "For substance " << substance.symbol() << ".";
            exception.line = __LINE__;
            RaiseError(exception)
        }

        if (j && thermo_parameters.phase_transition_prop[ft][0] <= TrK /*-C_to_K*/)
        {                                                               // Adding parameters of phase transition
            if (thermo_parameters.phase_transition_prop[ft].size() > 1) // dS
                S += thermo_parameters.phase_transition_prop[ft][1];
            if (thermo_parameters.phase_transition_prop[ft].size() > 2) // dH
                H += thermo_parameters.phase_transition_prop[ft][2];
            if (thermo_parameters.phase_transition_prop[ft].size() > 3) // dV
                V += thermo_parameters.phase_transition_prop[ft][3];
            // More to be added ?
            ft++;
        }

        G -= S * (TK - TrK);

        for (unsigned i = 0; i < thermo_parameters.Cp_coeff[j].size(); i++)
        {
            if (i == 16)
                break;
            ac[i] = thermo_parameters.Cp_coeff[j][i];
        }

        S += (ac[0] * log((TK / TrK)) +
                ac[1] * (TK - TrK) +
                ac[2] * (1. / Tst2 - 1. / (TK * TK)) / 2. +
                ac[3] * 2. * (1. / Tst05 - 1. / (pow(TK, 0.5))) +
                ac[4] * ((TK * TK) - Tst2) / 2. +
                ac[5] * ((pow(TK, 3)) - Tst3) / 3. +
                ac[6] * ((pow(TK, 4)) - Tst4) / 4. +
                ac[7] * (1. / Tst3 - 1. / (pow(TK, 3))) / 3. +
                ac[8] * (1. / TrK - 1. / TK) +
                ac[9] * 2. * ((pow(TK, 0.5)) - Tst05));

        G -= (ac[0] * (TK * log((TK / TrK)) - (TK - TrK)) +
                ac[1] * (pow((TK - TrK), 2)) / 2. +
                ac[2] * (pow((TK - TrK), 2)) / TK / Tst2 / 2. +
                ac[3] * 2. * ((pow(TK, 0.5)) - Tst05) * ((pow(TK, 0.5)) - Tst05) / Tst05 +
                ac[4] * ((pow(TK, 3)) + 2. * Tst3 - 3. * TK * Tst2) / 6. +
                ac[5] * ((pow(TK, 4)) + 3. * Tst4 - 4. * TK * Tst3) / 12. +
                ac[6] * ((pow(TK, 4)) * TK + 4. * Tst4 * TrK - 5. * TK * Tst4) / 20. +
                ac[7] * (Tst3 - 3. * (pow(TK, 2)) * TrK + 2. * (pow(TK, 3))) / 6. / (pow(TK, 2)) / Tst3 +
                ac[8] * ((TK / TrK) - 1. - log((TK / TrK))) +
                ac[9] * 2. * (2. * TK * (pow(TK, 0.5)) - 3. * TK * Tst05 + TrK * Tst05) / 3.);

        H += (ac[0] * (TK - TrK) +
                ac[1] * ((pow(TK, 2)) - Tst2) / 2. +
                ac[2] * (1. / TrK - 1. / TK) +
                ac[3] * 2. * ((pow(TK, 0.5)) - Tst05) +
                ac[4] * ((pow(TK, 3)) - Tst3) / 3. +
                ac[5] * ((pow(TK, 4)) - Tst4) / 4. +
                ac[6] * ((pow(TK, 4)) * TK - Tst4 * TrK) / 5 +
                ac[7] * (1. / Tst2 - 1. / (pow(TK, 2))) / 2. +
                ac[8] * log((TK / TrK)) +
                ac[9] * 2. * (TK * (pow(TK, 0.5)) - TrK * Tst05) / 3.);
    }

    thermo_properties_PT.heat_capacity_cp = Cp;
    thermo_properties_PT.gibbs_energy = G;
    thermo_properties_PT.enthalpy = H;
    thermo_properties_PT.entropy = S;
    thermo_properties_PT.volume = V;

    if (outsideBounds)
        *outsideBounds = k_outside_bounds;


    /// reaktoro implementation
    /*
    // Collect the temperature points used for the integrals along the pressure line P = Pr
    std::vector<real> Ti;

    const auto& Tr   = substance.referenceT();

    std::vector<double> dHt;
    std::vector<double> dVt;

    Ti.push_back(substance.referenceT());

    for(int i = 0; i < thermo_parameters.phase_transition_prop.size(); ++i)
    {
        if(TK_ > thermo_parameters.temperature_intervals[i][1])
        {Ti.push_back(thermo_parameters.temperature_intervals[i][1]);
        dHt.push_back(thermo_parameters.phase_transition_prop[i][2]);
        dVt.push_back(thermo_parameters.phase_transition_prop[i][3]);}
    }


    Ti.push_back(TK_);

    real xCp;
    for(unsigned i = 0; i+1 < Ti.size(); ++i)
        if(Ti[i] <= TK_ && TK_ <= Ti[i+1])
            xCp = thermo_parameters.Cp_coeff[i][0] + thermo_parameters.Cp_coeff[i][1]*TK_ + thermo_parameters.Cp_coeff[i][2]/(TK_*TK_);


    // Calculate the integrals of the heat capacity function of the mineral from Tr to T at constant pressure Pr
    real CpdT;
    real CpdlnT;
    for(unsigned i = 0; i+1 < Ti.size(); ++i)
    {
        const auto T0 = Ti[i];
        const auto T1 = Ti[i+1];

        for (unsigned j = 0; j < thermo_parameters.Cp_coeff[i].size(); j++)
        {
            if (j == 16)
                break;
            ac[j] = thermo_parameters.Cp_coeff[i][j];
        }


        CpdT += ac[0]*(T1 - T0) + 0.5*ac[1]*(T1*T1 - T0*T0) - ac[2]*(1.0/T1 - 1.0/T0);
        CpdlnT += ac[0]*log(T1/T0) + ac[1]*(T1 - T0) - 0.5*ac[2]*(1.0/(T1*T1) - 1.0/(T0*T0));
    }

    // Calculate the volume and other auxiliary quantities for the thermodynamic properties of the mineral
    real xV(0.0);
    real GdH;
    real HdH;
    real SdH;
    for(unsigned i = 1; i+1 < Ti.size(); ++i)
    {
        GdH += dHt[i-1]*(TK_ - Ti[i])/Ti[i];
        HdH += dHt[i-1];
        SdH += dHt[i-1]/Ti[i];

        xV += dVt[i-1];
    }

    // Calculate the standard molal thermodynamic properties of the mineral
    auto xG = Gr - Sr * (TK_ - Tr) + CpdT - TK_ * CpdlnT - GdH; // + VdP
    auto xH = Hr + CpdT + HdH;  // + VdP
    auto xS = Sr + CpdlnT + SdH;
//    auto xU = xH - Pb*V;
//    auto xA = xU - TK_*S;

    */

    return thermo_properties_PT;
}

} // namespace ThermoFun
