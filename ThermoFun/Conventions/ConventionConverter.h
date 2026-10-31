// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_CONVENTIONCONVERTER_H
#define THERMOFUN_CONVENTIONS_CONVENTIONCONVERTER_H

#include <cmath>
#include <functional>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Conventions/Convention.h>

namespace ThermoFun::v2 {

/// What a conversion needs to know about a substance.
struct ConversionSubject
{
    double Tr = 298.15;                          ///< reference temperature (K)
    bool gas = false;
    bool solute = false;
    std::function<double()> element_entropy;     ///< the sum of the entropies of the elements of the formula, J/(mol K). Called only if needed.
};

/// A change of convention of one substance:
///   G += G0 + G_per_K T,   S -= G_per_K,   H, Cp and V do not change
/// and, for each unit of `molarity`, the terms of molarityTerms. Sums of these changes give the change of a reaction.
struct ConventionShift
{
    double G0 = 0.0;        ///< J/mol
    double G_per_K = 0.0;   ///< J/(mol K)
    double molarity = 0.0;  ///< 1: from molarity to molality, -1: the other way

    auto identity() const -> bool { return G0 == 0.0 && G_per_K == 0.0 && molarity == 0.0; }
    auto needsSolvent() const -> bool { return molarity != 0.0; }
};

/// The change from the declared convention to the canonical one. A field without a value is canonical.
auto shiftToCanonical(const Convention& from, const ConversionSubject& subject) -> ConventionShift;

/// The change from the canonical convention to the requested one.
auto shiftFromCanonical(const Convention& to, const ConversionSubject& subject) -> ConventionShift;

/// The change of the properties of a solute from molarity to molality, in SI units:
///   G_m = G_c + R T L,  with L = ln(rho/1000) and rho the density of water in kg/m3.
/// S, H, Cp and V follow from G. The derivatives of Cp are not exact: they need third derivatives of rho.
template <typename Scalar>
struct MolarityTerms
{
    Scalar G, H, S, Cp, V;
};

/// The second derivative of the density of water by T, kg/(m3 K2). The field densityTT of the water models differs from the derivative of
/// densityT (3 % at 473 K); the derivative of densityT agrees with finite differences. Without autodiff that derivative is not there.
template <typename Properties>
auto densityTT(const Properties& solvent) -> double
{
    return solvent.densityT.ddt != 0.0 ? solvent.densityT.ddt : solvent.densityTT.val;
}

template <typename TScalar, typename Scalar>
auto molarityTerms(const TScalar& T, const Scalar& rho, const Scalar& rhoT, const Scalar& rhoP, const Scalar& rhoTT) -> MolarityTerms<Scalar>
{
    using std::log;
    const Scalar T_ = T;
    const Scalar L = log(rho / 1000.0);
    const Scalar LT = rhoT / rho;
    const Scalar LP = rhoP / rho;
    const Scalar LTT = rhoTT / rho - LT * LT;
    MolarityTerms<Scalar> m;
    m.G = R_CONSTANT * T_ * L;
    m.S = -R_CONSTANT * (L + T_ * LT);
    m.H = -R_CONSTANT * T_ * T_ * LT;
    m.Cp = -R_CONSTANT * (2.0 * T_ * LT + T_ * T_ * LTT);
    m.V = R_CONSTANT * T_ * LP;
    return m;
}

} // namespace ThermoFun::v2

#endif
