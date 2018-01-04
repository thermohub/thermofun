#ifndef THERMOMODELSUBSTANCE_H
#define THERMOMODELSUBSTANCE_H

#include <memory>

namespace ThermoFun {

struct Substance;
struct PropertiesSolvent;
struct ElectroPropertiesSolvent;
struct ThermoPropertiesSubstance;

/**
 * @brief The ThermoModelsSubstance class
 */
class ThermoModelsSubstance
{
public:
    /// Construct a ThermoModelsSubstance instance with given Sunstance instance
    explicit ThermoModelsSubstance(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The SoluteAkinfievDiamondEOS class is the implementation of the Akinfiev & Diamond EOS for neutral species
 * References: Akinfiev N.N. and Diamond L.W. Thermodynamic description of aqueous nonelectrolytes at infinite
 * dilution over a wide range of state parameters. GCA, 2003 67:613-629.
 */
class SoluteAkinfievDiamondEOS
{
public:
    /// Construct a default SoluteAkinfievDiamondEOS instance
    SoluteAkinfievDiamondEOS();

    /// Construct a SoluteAkinfievDiamondEOS instance from a Species instance
    explicit SoluteAkinfievDiamondEOS(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param tps thermodynamic properties of the substance previosuly corrected with the AD EOS
    /// @param wtp thermodynamic properties of liquid H2O
    /// @param wigp themrodynamic properties of water in ideal gas state
    /// @param wp water solvent proeoprties (e.g. density, alpha, beta, etc.)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance &wtp, const ThermoPropertiesSubstance &wigp, const PropertiesSolvent& wp, const ThermoPropertiesSubstance &wtpr, const ThermoPropertiesSubstance &wigpr, const PropertiesSolvent &wpr) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The WaterIdealGasWoolley class calculates the ideal gas properties of pure H2O
 * References: Woolley, H. W. Thermodynamic Properties for H2O in the Ideal Gas State; Straub, J. and K. Scheffler,
 * eds.,1980 p. 166, Pergamon: Oxford.
 */
class WaterIdealGasWoolley
{
public:
    /// Construct a default AqueousSpecies instance
    WaterIdealGasWoolley();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit WaterIdealGasWoolley(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The SoluteHKFgems class calculates the thermodynamic properties of aqueous species using the revised
 * Helgeson-Kirkham-Flowers EOS, as was implemented in GEM-Selektor
 * References: Tanger J.C. and Helgeson H.C. Calculation of the thermodynamic and transport properties of
 * aqueous species at high pressures and temperatures; revised equations of state for the standard partial
 * molal properties of ions and electrolytes. Am. J. Sci., 1988 288:19-98.
 * Johnson J.W., Oelkers E.H. and Helgeson H.C. SUPCRT92: A software package for calculating the standard
 * molal thermodynamic properties of minerals, gases, aqueous species, and reactions from 1 to 5000 bar
 * and 0 to 1000°C. Comp. Geosci., 1992 18:899-947.
 */
class SoluteHKFgems
{
public:
    /// Construct a default AqueousSpecies instance
    SoluteHKFgems();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit SoluteHKFgems(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param wp structure holding the water solvent porperties
    /// @param wes structure holding the water solvent electro-chemical properties
    auto thermoProperties (double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The SoluteHKFreaktoro class calculates the thermodynamic properties of aqueous species using the revised
 * Helgeson-Kirkham-Flowers EOS, as was implemented in Reaktoro
 * References: Tanger J.C. and Helgeson H.C. Calculation of the thermodynamic and transport properties of
 * aqueous species at high pressures and temperatures; revised equations of state for the standard partial
 * molal properties of ions and electrolytes. Am. J. Sci., 1988 288:19-98.
 * Johnson J.W., Oelkers E.H. and Helgeson H.C. SUPCRT92: A software package for calculating the standard
 * molal thermodynamic properties of minerals, gases, aqueous species, and reactions from 1 to 5000 bar
 * and 0 to 1000°C. Comp. Geosci., 1992 18:899-947.
 */
class SoluteHKFreaktoro
{
public:
    /// Construct a default AqueousSpecies instance
    SoluteHKFreaktoro();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit SoluteHKFreaktoro(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param wp structure holding the water solvent porperties
    /// @param wes structure holding the water solvent electro-chemical properties
    auto thermoProperties (double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The MinMurnaghanEOSHP98 class
 */
class MinMurnaghanEOSHP98
{
public:
    /// Construct a default AqueousSpecies instance
    MinMurnaghanEOSHP98();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit MinMurnaghanEOSHP98(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param tps thermodynamic properties of the substance previosuly corrected
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The MinBerman88 class
 * References: Berman RG (1988) Internally consistent thermodynamic data for minerals in the system
 * na2o-k2o-cao-mgo-feo-fe2o-sio2-tio2-h2o-co2. Journal of Petrology 29:445-552
 */
class MinBerman88
{
public:
    /// Construct a default MinBerman88 instance
    MinBerman88();

    /// Construct an MinBerman88 instance from a Substance instance
    explicit MinBerman88(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param tps thermodynamic properties of the substance previosuly corrected
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The MinBMGottschalk class
 */
class MinBMGottschalk
{
public:
    /// Construct a default MinBMGottschalk instance
    MinBMGottschalk();

    /// Construct an MinBMGottschalk instance from a Substance instance
    explicit MinBMGottschalk(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param tps thermodynamic properties of the substance previosuly corrected with the MinBMGottschalk
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};


/**
 * @brief The EmpiricalCpIntegration class calculates the head capacity using an empirical
 * heat capacity equation Cp=f(T);
 */
class EmpiricalCpIntegration
{
public:
    /// Construct a default EmpiricalCpIntegration instance
    EmpiricalCpIntegration();

    /// Construct an EmpiricalCpIntegration instance from a Substance instance
    explicit EmpiricalCpIntegration(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/**
 * @brief The HPLandau class does T and P corrections on the themrodynamic porperties for
 * phases with Landau transition
 * References: Holland T.J.B. and Powell R. (1998) An internally consistent thermodynamic data set for phases of
 * petrological interest. J. Metamorph. Geol. 16, 309-343.
 */
class HPLandau
{
public:
    /// Construct a default HPLandau instance
    HPLandau();

    /// Construct an HPLandau instance from a Substance instance
    explicit HPLandau(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param tps thermodynamic properties of the substance previosuly corrected
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
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
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
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
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
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
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
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
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
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
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
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class ConMolVol
{
public:
    /// Construct a default ConMolVol instance
    ConMolVol();

    /// Construct an ConMolVol instance from a Substance instance
    explicit ConMolVol(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

///
class IdealGasLawVol
{
public:
    /// Construct a default IdealGasLawVol instance
    IdealGasLawVol();

    /// Construct an IdealGasLawVol instance from a Substance instance
    explicit IdealGasLawVol(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // THERMOMODELSUBSTANCE_H
