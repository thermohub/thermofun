#ifndef THERMOMODELSUBSTANCE_H
#define THERMOMODELSUBSTANCE_H

// TCorrPT includes
#include "Substance.h"

namespace TCorrPT {

class ThermoModelsSubstance
{
public:
    /// Construct a ThermoModelsSubstance instance with given Sunstance instance
    explicit ThermoModelsSubstance(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class SoluteAkinfievDiamondEOS
{
public:
    /// Construct a default AqueousSpecies instance
    SoluteAkinfievDiamondEOS();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit SoluteAkinfievDiamondEOS(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param tps thermodynamic properties of the substance previosuly corrected with the AD EOS
    /// @param wtp thermodynamic properties of liquid H2O
    /// @param wigp themrodynamic properties of water in ideal gas state
    /// @param wp water solvent proeoprties (e.g. density, alpha, beta, etc.)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance &wtp, const ThermoPropertiesSubstance &wigp, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class WaterIdealGasWoolley
{
public:
    /// Construct a default AqueousSpecies instance
    WaterIdealGasWoolley();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit WaterIdealGasWoolley(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class SoluteHKFgems
{
public:
    /// Construct a default AqueousSpecies instance
    SoluteHKFgems();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit SoluteHKFgems(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param wp structure holding the water solvent porperties
    /// @param wes structure holding the water solvent electro-chemical properties
    auto thermoProperties (double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};


class SoluteHKFreaktoro
{
public:
    /// Construct a default AqueousSpecies instance
    SoluteHKFreaktoro();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit SoluteHKFreaktoro(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param wp structure holding the water solvent porperties
    /// @param wes structure holding the water solvent electro-chemical properties
    auto thermoProperties (double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class MinMurnaghanEOSHP98
{
public:
    /// Construct a default AqueousSpecies instance
    MinMurnaghanEOSHP98();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit MinMurnaghanEOSHP98(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param tps thermodynamic properties of the substance previosuly corrected with the Murnaghan EOS
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class MinBerman88
{
public:
    /// Construct a default MinBerman88 instance
    MinBerman88();

    /// Construct an MinBerman88 instance from a Substance instance
    explicit MinBerman88(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param tps thermodynamic properties of the substance previosuly corrected with the MinBerman88
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class MinBMGottschalk
{
public:
    /// Construct a default MinBMGottschalk instance
    MinBMGottschalk();

    /// Construct an MinBMGottschalk instance from a Substance instance
    explicit MinBMGottschalk(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param tps thermodynamic properties of the substance previosuly corrected with the MinBMGottschalk
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};


/// integration of empirical heat capacity equation Cp=f(T);
class EmpiricalCpIntegration
{
public:
    /// Construct a default EmpiricalCpIntegration instance
    EmpiricalCpIntegration();

    /// Construct an EmpiricalCpIntegration instance from a Substance instance
    explicit EmpiricalCpIntegration(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class HPLandau
{
public:
    /// Construct a default HPLandau instance
    HPLandau();

    /// Construct an HPLandau instance from a Substance instance
    explicit HPLandau(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class GasCORK
{
public:
    /// Construct a default GasCORK instance
    GasCORK();

    /// Construct an GasCORK instance from a Substance instance
    explicit GasCORK(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class GasPRSV
{
public:
    /// Construct a default GasPRSV instance
    GasPRSV();

    /// Construct an GasPRSV instance from a Substance instance
    explicit GasPRSV(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class GasCGF
{
public:
    /// Construct a default GasCGF instance
    GasCGF();

    /// Construct an GasCGF instance from a Substance instance
    explicit GasCGF(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class GasSRK
{
public:
    /// Construct a default GasSRK instance
    GasSRK();

    /// Construct an GasSRK instance from a Substance instance
    explicit GasSRK(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class GasPR78
{
public:
    /// Construct a default GasPR78 instance
    GasPR78();

    /// Construct an GasPR78 instance from a Substance instance
    explicit GasPR78(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class GasSTP
{
public:
    /// Construct a default GasSTP instance
    GasSTP();

    /// Construct an GasSTP instance from a Substance instance
    explicit GasSTP(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // THERMOMODELSUBSTANCE_H
