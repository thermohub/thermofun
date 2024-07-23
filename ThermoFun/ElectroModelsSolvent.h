#ifndef ELECTROMODELSSOLVENT_H
#define ELECTROMODELSSOLVENT_H

#include <memory>

namespace ThermoFun {

class Substance;
struct ElectroPropertiesSolvent;
struct PropertiesSolvent;

/**
 * @brief The WaterJNreaktoro class calculates the electro-chemical proeprties of water solvent
 * using the Jhonson and Norton (1991) model as implemented in Reaktoro
 * References: Critical phenomena in hydrothermal systems; state, thermodynamic, electrostatic, and
 * transport properties of H2O in the critical region. Am J Sci, 1991 291:541-648;
 */
class WaterJNreaktoro
{
public:
    /// Construct a default class object instance
    WaterJNreaktoro();

    /// Construct an class object instance from a Species instance
    explicit WaterJNreaktoro(const Substance& substance);

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param ps structure holding the solvent properties
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default 0
    auto electroPropertiesSolvent(double T, double P, PropertiesSolvent ps, int state=0) -> ElectroPropertiesSolvent;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The WaterJNgems class calculates the electro-chemical properties of water solvent
 * using the Jhonson and Norton (1991) model as implemented in GEMS.
 * References: Critical phenomena in hydrothermal systems; state, thermodynamic, electrostatic, and
 * transport properties of H2O in the critical region. Am J Sci, 1991 291:541-648;
 */
class WaterJNgems
{
public:
    /// Construct a default class object instance
    WaterJNgems();

    /// Construct an class object instance from a Species instance
    explicit WaterJNgems(const Substance& substance);

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default 0
    auto electroPropertiesSolvent(double T, double P, int state=0) -> ElectroPropertiesSolvent;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The WaterElectroSverjensky2014 class calculates the electro-chemical properties of water solvent
 * using the Sverjensky et al. (2014) dielectric constant model should be used together with the
 * Zhang and Duan (2002) water PVT model
 * References: Sverjensky et al. Water in the deep Earth: The dielectric constant and the solubilities
 * of quartz and corundum to 60 kb and 1200 °C. GCA, 2014 129:125-145
 * Zhang and Duan Prediction of the PVT properties of water over wide range of temperatures and pressures
 * from molecular dynamics simulation. PEPI, 2002 149:335-354
 */
class WaterElectroSverjensky2014
{
public:
    /// Construct a default class object instance
    WaterElectroSverjensky2014();

    /// Construct an class object instance from a Species instance
    explicit WaterElectroSverjensky2014(const Substance& substance);

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param ps structure holding the solvent properties
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default -1 determined by the record key "aggregate_state" in the database "4": "AS_AQUEOUS" or "O": "AS_GAS"
    auto electroPropertiesSolvent(double T, double P, int state = -1) -> ElectroPropertiesSolvent;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The WaterElectroFernandez1997 class calculates the electro-chemical properties of water solvent
 * using the Fernandez et al. (1997) dielectric constant model
 * References: D.P. Fernandez, A.R.H. Goodwin, E.W. Lemmon, J.M.H.L.Sengers, and R.C. Williams, 1997,
 * A Formulation for the Static Permittivity of Water and Steam at Temperatures from 238 K to 873 K at Pressures
 * up to 1200 MPa, Including Derivatives and Debye–Hückel Coefficients, J. Phys. Chem. Ref. Data 26, 1125.
 */
class WaterElectroFernandez1997
{
public:
    /// Construct a default class object instance
    WaterElectroFernandez1997();

    /// Construct an class object instance from a Species instance
    explicit WaterElectroFernandez1997(const Substance& substance);

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param ps structure holding the solvent properties
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default -1 determined by the record key "aggregate_state" in the database "4": "AS_AQUEOUS" or "O": "AS_GAS"
    auto electroPropertiesSolvent(double T, double P, int state = -1) -> ElectroPropertiesSolvent;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // ELECTROMODELSSOLVENT_H
