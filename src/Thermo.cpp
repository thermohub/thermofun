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

} // namespace TCorrPT
