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

    if (subst.substanceClass() != SubstanceClass::type::AQSOLVENT)
    {
        switch( method_genEOS )
        {
            case MethodGenEoS_Thrift::type::CTPM_CPT:
            {
                EmpiricalCpIntegration CpInt ( subst );
                return CpInt.thermoProperties(T, P);
                break;
            }
        }

        // Exception
        errorMethodNotFound("substance", subst.name(), __LINE__);
    }

    if (subst.substanceClass() == SubstanceClass::type::AQSOLVENT)
    {
        switch(method_T)
        {
            case MethodCorrT_Thrift::type::CTM_WAT:
            {
                WaterHGK water ( subst );
                return water.thermoPropertiesSubstance(T, P);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WAR:
            {
                WaterHGKreaktoro water ( subst );
                return water.thermoPropertiesSubstance(T, P);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WWP:
            {
                WaterWP95reaktoro water ( subst );
                return water.thermoPropertiesSubstance(T, P);
                break;
            }
        }

        // Exception
        errorMethodNotFound("substance", subst.name(), __LINE__);
    }

    // Exception
    errorMethodNotFound("substance", subst.name(), __LINE__);

   ThermoPropertiesSubstance tps;
   return tps;
}


auto Thermo::electroPropertiesSolvent(double T, double P, std::string substance) -> ElectroPropertiesSolvent
{
    Substance subst = pimpl->database.getSubstance(substance);
    MethodCorrT_Thrift::type  method_T      = subst.method_T();

    if (subst.substanceClass() == SubstanceClass::type::AQSOLVENT)
    {
        switch(method_T)
        {
            case MethodCorrT_Thrift::type::CTM_WAT:
            {
                WaterHGK water ( subst );
                return water.electroPropertiesSolvent(T, P);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WAR:
            {
                WaterHGKreaktoro water ( subst );
                return water.electroPropertiesSolvent(T, P);
                break;
            }
        }

        // Exception
        errorMethodNotFound("solvent", subst.name(), __LINE__);
    }

    // Exception
    errorMethodNotFound("solvent", subst.name(), __LINE__);

   ElectroPropertiesSolvent tps;
   return tps;
}

auto Thermo::propertiesSolvent(double T, double P, std::string solvent) -> PropertiesSolvent
{
    Substance subst = pimpl->database.getSubstance(solvent);
    MethodCorrT_Thrift::type  method_T      = subst.method_T();

    if (subst.substanceClass() == SubstanceClass::type::AQSOLVENT)
    {
        switch(method_T)
        {
            case MethodCorrT_Thrift::type::CTM_WAT:
            {
                WaterHGK water ( subst );
                return water.propertiesSolvent(T, P);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WAR:
            {
                WaterHGKreaktoro water ( subst );
                return water.propertiesSolvent(T, P);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WWP:
            {
                WaterWP95reaktoro water ( subst );
                return water.propertiesSolvent(T, P);
                break;
            }
        }

        // Exception
        errorMethodNotFound("solvent", subst.name(), __LINE__);
    }

    // Exception
    errorMethodNotFound("solvent", subst.name(), __LINE__);

   PropertiesSolvent tps;
   return tps;
}

} // namespace TCorrPT
