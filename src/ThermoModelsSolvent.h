#ifndef THERMOMODELSSOLVENT_H
#define THERMOMODELSSOLVENT_H

#include <memory>

namespace ThermoFun {

// forward declarations
struct Substance;
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;

/**
 * @brief The WaterHGK class calculates the properties of water using the Haar-Gallagher-Kell (1984)
 * equation of state, as was implemented in GEM-Selektor
 * References: HAAR L., GALLAGHER J. S., and KELL G. S. Steam Tables, Thermodynamic and Transport Properties
 * and Computer Programs for Vapor and Liquid States of Wafer in SI Unites. 1984, Hemisphere Publishing Co.
 */
class WaterHGK
{
public:
    /// Construct a default class object instance
    WaterHGK();

    /// Construct an class object instance from a Species instance
    explicit WaterHGK(const Substance& substance);

    /// Return the physical properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto propertiesSolvent (double T, double &P, int state) -> PropertiesSolvent;

    /// Return the themrodynamic properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The WaterHGKreaktoro class calculates the properties of water using the Haar-Gallagher-Kell (1984)
 * equation of state as implemented in Reaktoro.
 * References: HAAR L., GALLAGHER J. S., and KELL G. S. Steam Tables, Thermodynamic and Transport Properties
 * and Computer Programs for Vapor and Liquid States of Wafer in SI Unites. 1984, Hemisphere Publishing Co.
 */
class WaterHGKreaktoro
{
public:
    /// Construct a default class object instance
    WaterHGKreaktoro();

    /// Construct an class object instance from a Species instance
    explicit WaterHGKreaktoro(const Substance& substance);

    /// Return the physical properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto propertiesSolvent (double T, double &P, int state) -> PropertiesSolvent;

    /// Return the themrodynamic properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The WaterWP95reaktoro class
 * References: Wagner, W., Prub, A. The IAPWS formulation 1995 for the thermodynamic properties of ordinary
 * water substance for general and scientific use. J. Phys. Chem. Ref. Data, 2002 31(2):387–535.
 */
class WaterWP95reaktoro
{
public:
    /// Construct a default class object instance
    WaterWP95reaktoro();

    /// Construct an class object instance from a Species instance
    explicit WaterWP95reaktoro(const Substance& substance);

    /// Return the physical properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto propertiesSolvent (double T, double &P, int state) -> PropertiesSolvent;

    /// Return the themrodynamic properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The WaterZhangDuan2005 class calculates the properties of water using the Zhang and Duan (2005) equation of state
 * References: Zhang and Duan Prediction of the PVT properties of water over wide range of temperatures and pressures
 * from molecular dynamics simulation. PEPI, 2002 149:335-354
 */
class WaterZhangDuan2005
{
public:
    /// Construct a default class object instance
    WaterZhangDuan2005();

    /// Construct an class object instance from a Species instance
    explicit WaterZhangDuan2005(const Substance& substance);

    /// Return the physical properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto propertiesSolvent (double T, double &P, int state) -> PropertiesSolvent;

    /// Return the themrodynamic properties of the solvent.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param state defines the state of the solvent (liquid 0, vapor 1, below the critical point)
    auto thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // THERMOMODELSSOLVENT_H
