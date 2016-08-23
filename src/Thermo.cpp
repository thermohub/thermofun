// TCorrPT includes
#include "Common/Exception.h"
#include "Thermo.h"

namespace TCorrPT {

struct Thermo::Impl
{
    /// The database instance
    Database database;

    Impl()
    {}

    Impl(const Database& database)
    : database(database)
    {}
};

Thermo::Thermo(const Database& database)
: pimpl(new Impl(database))
{}


auto Thermo::thermoPropertiesSubstance(double T, double &P, std::string substance) -> ThermoPropertiesSubstance
{
    Substance subst = pimpl->database.getSubstance(substance);
    MethodGenEoS_Thrift::type method_genEOS = subst.methodGenEOS();
    MethodCorrT_Thrift::type  method_T      = subst.method_T();
    MethodCorrP_Thrift::type  method_P      = subst.method_P();
    ThermoPropertiesSubstance tps;
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

    int solvent_state = 0; // default liquid (0), gas/vapor (1)

    if (subst.name() == "H+")
    {
        return tps;
    }

    if (subst.substanceClass() != SubstanceClass::type::AQSOLVENT)
    {
        // metohd EOS
        switch( method_genEOS )
        {
        case MethodGenEoS_Thrift::type::CTPM_CPT:
        {
            EmpiricalCpIntegration CpInt ( subst );
            tps = CpInt.thermoProperties(T, P);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_HKF:
        {
            SoluteHKFgems aqHKF( subst );
            string waterSolventSymbol = subst.SolventSymbol();
            if (!waterSolventSymbol.empty())
            {
               ElectroPropertiesSolvent wes = electroPropertiesSolvent(T, P, waterSolventSymbol);
               PropertiesSolvent        wp = propertiesSolvent(T, P, waterSolventSymbol);
                                        tps = aqHKF.thermoProperties(T, P, wp, wes);
            } else
            {
                errorSolventNotDefined("solvent", subst.symbol(), __LINE__);
            }
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_HKFR:
        {
            SoluteHKFreaktoro aqHKF( subst );
            string waterSolventSymbol = subst.SolventSymbol();
            if (!waterSolventSymbol.empty())
            {
               ElectroPropertiesSolvent wes = electroPropertiesSolvent(T, P, waterSolventSymbol);
               PropertiesSolvent        wp = propertiesSolvent(T, P, waterSolventSymbol);
                                        tps = aqHKF.thermoProperties(T, P, wp, wes);
            } else
            {
                errorSolventNotDefined("solvent", subst.symbol(), __LINE__);
            }
            break;
        }
            // Exception
            errorMethodNotFound("substance", subst.symbol(), __LINE__);
        }

        // method T
        switch ( method_T )
        {
        case MethodCorrT_Thrift::type::CTM_CHP:
        {
            HPLandau hpLandau ( subst );
            tps = hpLandau.thermoProperties(T, P, tps);
            break;
        }
            // Exception
            errorMethodNotFound("substance", subst.symbol(), __LINE__);
        }

        // method P
        switch ( method_P )
        {
        case MethodCorrP_Thrift::type::CPM_AKI:
        {
            SoluteAkinfievDiamondEOS aqAD (subst);
            string waterSolventSymbol = subst.SolventSymbol();
            if (!waterSolventSymbol.empty())
            {
                PropertiesSolvent           wp      = propertiesSolvent(T, P, waterSolventSymbol);
                ThermoPropertiesSubstance   wtp     = thermoPropertiesSubstance(T, P, waterSolventSymbol);
                WaterIdealGasWoolley        H2Oig   (pimpl->database.getSubstance(waterSolventSymbol));
                ThermoPropertiesSubstance   wig     = H2Oig.thermoProperties(T, P);
                tps     = aqAD.thermoProperties(T, P, tps, wtp, wig, wp);
            } else
            {
                errorSolventNotDefined("solvent", subst.symbol(), __LINE__);
            }
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CEH:
        {
            MinMurnaghanEOSHP98 minHP (subst);
            tps = minHP.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_VBE:
        {
            MinBerman88 minBM (subst);
            tps = minBM.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_VBM:
        {
            MinBMGottschalk minBMG (subst);
            tps = minBMG.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CORK:
        {
            GasCORK gasCORK (subst);
            tps = gasCORK.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_PRSV:
        {
            GasPRSV gasPRSV (subst);
            tps = gasPRSV.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_EMP:
        {
            GasCGF gasCGF (subst);
            tps = gasCGF.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_SRK:
        {
            GasSRK gasSRK (subst);
            tps = gasSRK.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_PR78:
        {
            GasPR78 gasPR78 (subst);
            tps = gasPR78.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_STP:
        {
            GasSTP gasSTP (subst);
            tps = gasSTP.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CON: // Molar volume assumed independent of T and P
        {
            ThermoPropertiesSubstance rtps = subst.thermoReferenceProperties();

            tps.volume           = rtps.volume;
            tps.gibbs_energy    += rtps.volume * (p - (subst.referenceP() / bar_to_Pa));
            tps.enthalpy        += rtps.volume * (p - (subst.referenceP() / bar_to_Pa));
            tps.internal_energy  = tps.enthalpy - p*tps.volume;
            tps.helmholtz_energy = tps.internal_energy - (t)*tps.entropy;

            break;
        }
        case MethodCorrP_Thrift::type::CPM_OFF:
        {
            if(( subst.substanceClass() == SubstanceClass::type::GASFLUID ) && P > 0.0 )
            { // molar volume from the ideal gas law
                tps.volume = (t) / p * R_CONSTANT;
            }
            break;
        }
            // Exception
            errorMethodNotFound("substance", subst.symbol(), __LINE__);
        }
    }

    if (subst.substanceClass() == SubstanceClass::type::AQSOLVENT)
    {
        if (method_P == MethodCorrP_Thrift::type::CPM_GAS) solvent_state = 1;
        switch(method_T)
        {
        case MethodCorrT_Thrift::type::CTM_WAT:
        {
            WaterHGK water ( subst );
            tps = water.thermoPropertiesSubstance(T, P, solvent_state);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WAR:
        {
            WaterHGKreaktoro water ( subst );
            tps = water.thermoPropertiesSubstance(T, P, solvent_state);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WWP:
        {
            WaterWP95reaktoro water ( subst );
            tps = water.thermoPropertiesSubstance(T, P, solvent_state);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WZD:
        {
            WaterZhangDuan2005 water ( subst );
            tps = water.thermoPropertiesSubstance(T, P, solvent_state);
            break;
        }
            // Exception
            errorMethodNotFound("substance", subst.symbol(), __LINE__);
        }
    }
   return tps;
}


auto Thermo::electroPropertiesSolvent(double T, double &P, std::string substance) -> ElectroPropertiesSolvent
{
    Substance subst = pimpl->database.getSubstance(substance);
//    MethodCorrT_Thrift::type  method_T      = subst.method_T();
    MethodGenEoS_Thrift::type method_genEOS = subst.methodGenEOS();
//    MethodCorrP_Thrift::type  method_P      = subst.method_P();
    PropertiesSolvent ps;
    ElectroPropertiesSolvent eps;
//    int solvent_state = 0; // default liquid (0), gas/vapor (1)

    if (subst.substanceClass() == SubstanceClass::type::AQSOLVENT)
    {
        switch(method_genEOS)
        {
        case MethodGenEoS_Thrift::type::CEM_WJNR:
        {
            WaterJNreaktoro water (subst);
            ps = propertiesSolvent(T, P, subst.symbol());
            eps = water.electroPropertiesSolvent(T, P, ps);;
            break;
        }
        case MethodGenEoS_Thrift::type::CEM_WJNG:
        {
            WaterJNgems water (subst);
            ps = propertiesSolvent(T, P, subst.symbol());
            eps = water.electroPropertiesSolvent(T, P);
            break;
        }
        case MethodGenEoS_Thrift::type::CEM_WSV14:
        {
            WaterElectroSverjensky2014 water (subst);
            ps = propertiesSolvent(T, P, subst.symbol());
            eps = water.electroPropertiesSolvent(T, P, ps);
            break;
        }
            // Exception
            errorMethodNotFound("solvent", subst.symbol(), __LINE__);
        }
    }

   return eps;
}

auto Thermo::propertiesSolvent(double T, double &P, std::string solvent) -> PropertiesSolvent
{
    Substance subst = pimpl->database.getSubstance(solvent);
    MethodCorrT_Thrift::type  method_T      = subst.method_T();
    MethodCorrP_Thrift::type  method_P      = subst.method_P();
    PropertiesSolvent ps;
    int solvent_state = 0; // default liquid (0), gas/vapor (1)

    if (subst.substanceClass() == SubstanceClass::type::AQSOLVENT)
    {
        if (method_P == MethodCorrP_Thrift::type::CPM_GAS) solvent_state = 1;
        switch(method_T)
        {
        case MethodCorrT_Thrift::type::CTM_WAT:
        {
            WaterHGK water ( subst );
            ps = water.propertiesSolvent(T, P, solvent_state);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WAR:
        {
            WaterHGKreaktoro water ( subst );
            ps = water.propertiesSolvent(T, P, solvent_state);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WWP:
        {
            WaterWP95reaktoro water ( subst );
            ps = water.propertiesSolvent(T, P, solvent_state);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WZD:
        {
            WaterZhangDuan2005 water ( subst );
            ps = water.propertiesSolvent(T, P, solvent_state);
            break;
        }
            // Exception
            errorMethodNotFound("solvent", subst.symbol(), __LINE__);
        }
    }
   return ps;
}

} // namespace TCorrPT
