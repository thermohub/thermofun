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


auto Thermo::thermoProperties(double T, double P, std::string substance) -> ThermoPropertiesSubstance
{
    Substance subst = pimpl->database.getSubstance(substance);
    MethodGenEoS_Thrift::type method_genEOS = subst.methodGenEOS();

    switch( method_genEOS )
    {
        case MethodGenEoS_Thrift::type::CTPM_CPT:
        {
            EmpiricalCpIntegration CpInt ( subst );
            return CpInt.thermoProperties(T, P);
            break;
        }
    }
}

} // namespace TCorrPT
