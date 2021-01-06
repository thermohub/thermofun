#include "EmpiricalCpIntegration.h"
#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Substance.h"
#include <iomanip>

namespace ThermoFun
{

auto thermoPropertiesEmpCpIntegration(Reaktoro_::Temperature TK, Reaktoro_::Pressure /*Pbar*/, Substance substance) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance thermo_properties_PT = substance.thermoProperties();
    ThermoPropertiesSubstance thermo_properties_PrTr = substance.thermoReferenceProperties();
    SubstanceClass::type substance_class = substance.substanceClass();
    ThermoParametersSubstance thermo_parameters = substance.thermoParameters();

    Reaktoro_::ThermoScalar V;
    int k = -1;
    vector<double> ac;
    auto TK_ = TK;
    for (unsigned i = 0; i < 16; i++)
    {
        ac.push_back(0.0);
    }

    auto TrK = substance.referenceT() /* + C_to_K*/;

    auto Sr = thermo_properties_PrTr.entropy;
    auto Gr = thermo_properties_PrTr.gibbs_energy;
    auto Hr = thermo_properties_PrTr.enthalpy;

    auto S = thermo_properties_PrTr.entropy;
    auto G = thermo_properties_PrTr.gibbs_energy;
    auto H = thermo_properties_PrTr.enthalpy;

    if (thermo_parameters.Cp_coeff.size() == 0)
    {
        errorModelParameters("Cp empirical coefficients", "empicrical Cp integration", __LINE__, __FILE__);
        return thermo_properties_PrTr;
    }

    // get Cp interval -> this has to go!!!!
    for (size_t i = 0; i < thermo_parameters.temperature_intervals.size(); i++)
    {
        if (thermo_parameters.temperature_intervals[i].size() > 0)
        {
            if ((thermo_parameters.temperature_intervals[i][0] <= TK) && (thermo_parameters.temperature_intervals[i][1] > TK))
            {
                k = i;
                break;
            }
        }
        else
        {
            k = 0;
        }
    }

    if (k < 0)
    {
        if (TK_ < thermo_parameters.temperature_intervals[0][0])
            k = 0;
        if (TK_ > thermo_parameters.temperature_intervals[thermo_parameters.temperature_intervals.size() - 1][1])
            k = thermo_parameters.temperature_intervals.size() - 1;
        std::cout << "The given temperature: " << TK_ << " is not inside the specified interval/s for the Cp calculation.";
        std::cout << "The temperature is not inside the specified interval for the substance " << substance.symbol() << "." << std::endl
                  << __FILE__ << std::endl
                  << __LINE__;
    }

    //k = 0; fix

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
            TK = TK_.val /* + C_to_K*/; // current T is the end T for phase transition Cp calculations
        else
            TK = thermo_parameters.temperature_intervals[j][1] /*+ C_to_K*/; // takes the upper bound from the j-th Tinterval

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

    if (k < 0)
    {
        setMessage(Reaktoro_::Status::calculated, "Empirical Cp integration: Outside temperature bounds", thermo_properties_PT);
    }


    /// reaktoro implementation
    /*
    // Collect the temperature points used for the integrals along the pressure line P = Pr
    std::vector<Reaktoro_::Temperature> Ti;

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

    Reaktoro_::ThermoScalar xCp;
    for(unsigned i = 0; i+1 < Ti.size(); ++i)
        if(Ti[i] <= TK_ && TK_ <= Ti[i+1])
            xCp = thermo_parameters.Cp_coeff[i][0] + thermo_parameters.Cp_coeff[i][1]*TK_ + thermo_parameters.Cp_coeff[i][2]/(TK_*TK_);


    // Calculate the integrals of the heat capacity function of the mineral from Tr to T at constant pressure Pr
    Reaktoro_::ThermoScalar CpdT;
    Reaktoro_::ThermoScalar CpdlnT;
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
    Reaktoro_::ThermoScalar xV(0.0);
    Reaktoro_::ThermoScalar GdH;
    Reaktoro_::ThermoScalar HdH;
    Reaktoro_::ThermoScalar SdH;
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
