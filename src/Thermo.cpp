// TCorrPT includes
#include "Common/Exception.h"
#include "Thermo.h"

namespace TCorrPT {


auto errorMethodNotFound(std::string type, std::string name, int line) -> void
{
    Exception exception;
    exception.error << "The calculation method was not found.";
    exception.reason << "The calculation method defined for the " << type << " " << name << " is not implemented.";
    exception.line = line;
    RaiseError(exception);
}


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


auto Thermo::thermoPropertiesSubstance(double T, double P, std::string substance) -> ThermoPropertiesSubstance
{
    Substance subst = pimpl->database.getSubstance(substance);
    MethodGenEoS_Thrift::type method_genEOS = subst.methodGenEOS();
    MethodCorrT_Thrift::type  method_T      = subst.method_T();
    MethodCorrP_Thrift::type  method_P      = subst.method_P();
    ThermoPropertiesSubstance tps;
    int solvent_state = 0; // default liquid (0), gas/vapor (1)

    if (subst.substanceClass() != SubstanceClass::type::AQSOLVENT)
    {
        // metohd EOS
        switch( method_genEOS )
        {
        case MethodGenEoS_Thrift::type::CTPM_CPT:
        {
            EmpiricalCpIntegration CpInt ( subst );
//                return CpInt.thermoProperties(T, P);
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
               PropertiesSolvent wp = propertiesSolvent(T, P, waterSolventSymbol);
//                   return aqHKF.thermoProperties(T, P, wp, wes);
               tps = aqHKF.thermoProperties(T, P, wp, wes);
            } else
            {
                Exception exception;
                exception.error << "Solvent symbol not defiend";
                exception.reason << "The solvent symbol for " << subst.name() <<  " was not defined.";
                exception.line = __LINE__;
                RaiseError(exception);
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
               PropertiesSolvent wp = propertiesSolvent(T, P, waterSolventSymbol);
//                   return aqHKF.thermoProperties(T, P, wp, wes);
               tps = aqHKF.thermoProperties(T, P, wp, wes);
            } else
            {
                Exception exception;
                exception.error << "Solvent symbol not defiend";
                exception.reason << "The solvent symbol for " << subst.name() <<  " was not defined.";
                exception.line = __LINE__;
                RaiseError(exception);
            }
            break;
        }
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
                    PropertiesSolvent wp = propertiesSolvent(T, P, waterSolventSymbol);
                    ThermoPropertiesSubstance wtp = thermoPropertiesSubstance(T, P, waterSolventSymbol);
                    WaterIdealGasWoolley H2Oig (pimpl->database.getSubstance(waterSolventSymbol));
                    ThermoPropertiesSubstance wig = H2Oig.thermoProperties(T, P);
                    tps = aqAD.thermoProperties(T, P, tps, wtp, wig, wp);
                } else
                {
                    // error
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
//                return water.thermoPropertiesSubstance(T, P, solvent_state);
                tps = water.thermoPropertiesSubstance(T, P, solvent_state);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WAR:
            {
                WaterHGKreaktoro water ( subst );
//                return water.thermoPropertiesSubstance(T, P, solvent_state);
                tps = water.thermoPropertiesSubstance(T, P, solvent_state);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WWP:
            {
                WaterWP95reaktoro water ( subst );
//                return water.thermoPropertiesSubstance(T, P, solvent_state);
                tps = water.thermoPropertiesSubstance(T, P, solvent_state);
                break;
            }
            // Exception
            errorMethodNotFound("substance", subst.symbol(), __LINE__);
        }
    }

   return tps;
}


auto Thermo::electroPropertiesSolvent(double T, double P, std::string substance) -> ElectroPropertiesSolvent
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
//        if (method_P == MethodCorrP_Thrift::type::CPM_GAS) solvent_state = 1;
//        switch(method_T)
//        {
//            case MethodCorrT_Thrift::type::CTM_WAT:
//            {
//                WaterHGK water ( subst );
//                ps =  water.propertiesSolvent(T, P, solvent_state);
//                break;
//            }
//            case MethodCorrT_Thrift::type::CTM_WAR:
//            {
//                WaterHGKreaktoro water ( subst );
//                ps =  water.propertiesSolvent(T, P, solvent_state);
//                break;
//            }
//            case MethodCorrT_Thrift::type::CTM_WWP:
//            {
//                WaterWP95reaktoro water ( subst );
//                ps =  water.propertiesSolvent(T, P, solvent_state);
//                break;
//            }
//            // Exception
//            errorMethodNotFound("solvent", subst.name(), __LINE__);
//        }

        switch(method_genEOS)
        {
            case MethodGenEoS_Thrift::type::CEM_WJNR:
            {
                WaterJNreaktoro water (subst);
//                return water.electroPropertiesSolvent(T, P, ps);
                ps = propertiesSolvent(T, P, subst.symbol());
                eps = water.electroPropertiesSolvent(T, P, ps);;
                break;
            }
            case MethodGenEoS_Thrift::type::CEM_WJNG:
            {
                WaterJNgems water (subst);
//                return water.electroPropertiesSolvent(T, P);
                ps = propertiesSolvent(T, P, subst.symbol());
                eps = water.electroPropertiesSolvent(T, P);
                break;
            }
            // Exception
            errorMethodNotFound("solvent", subst.symbol(), __LINE__);
        }
    }

   return eps;
}

auto Thermo::propertiesSolvent(double T, double P, std::string solvent) -> PropertiesSolvent
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
//                return water.propertiesSolvent(T, P, solvent_state);
                ps = water.propertiesSolvent(T, P, solvent_state);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WAR:
            {
                WaterHGKreaktoro water ( subst );
//                return water.propertiesSolvent(T, P, solvent_state);
                ps = water.propertiesSolvent(T, P, solvent_state);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WWP:
            {
                WaterWP95reaktoro water ( subst );
//                return water.propertiesSolvent(T, P, solvent_state);
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
