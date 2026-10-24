# 13 — External thermodynamic data sources: compatibility and methods ThermoFun could cover

Survey of thermodynamic data sources beyond the ThermoHub databases ThermoFun already ships (aq17, slop98,
cemdata18, psinagra-12-07, mines16, heracles), what each provides, what ThermoFun would need to use it, and which
new methods would widen its coverage. Adapters that read external formats belong in `thermohub-tools` (D34, D42);
ThermoFun's part is the **methods and conventions** the data needs. Facts marked *(check)* come from memory or
secondary sources and must be verified against the primary source before implementation.

---

## 1. Summary

| Source | Kind | What ThermoFun needs | Effort | Value |
|---|---|---|---|---|
| PHREEQC databases (phreeqc.dat, llnl.dat, Thermoddem, NEA TDB, THEREDA, …) | log K(T) of reactions + molar volumes | analytic log K with **log10 T** term (coefficient conversion), van 't Hoff, **tabulated log K** at 8 temperatures (llnl) | low–medium | very high (most used geochemical data) |
| LLNL thermo.dat / EQ3/6 | log K at 8 T on saturation curve | **tabulated / interpolated log K** (legacy id 8 `logk_lagrange_interp`, not implemented today) | medium | high |
| NEA TDB (electronic database) | critically evaluated ΔfG, ΔfH, S, Cp, log K with uncertainties | existing methods; uncertainty conventions (k = 2 → u) | low | high (reference quality, D9 uncertainties) |
| Thermoddem (BRGM), ThermoChimie, THEREDA | evaluated data for clays, cements, saline systems; PHREEQC/GWB files | as PHREEQC; THEREDA activity models out of scope | low–medium | high |
| SUPCRTBL, Holland–Powell ds62/ds633 | minerals with **modified Tait EoS** (HP2011), Landau 2011, Pitzer–Sterner gases | **`HP11Tait` (listed, not implemented)**, HP2011 Landau, ds62 file adapter | medium | high (petrology, high P) |
| DEW (Deep Earth Water) | HKF species to 60 kbar, 1200 °C | water: `WaterZhangDuan05` + `Sverjensky14Diel` (both exist); check DEW water G function and ω estimates | low–medium | high (deep fluids) |
| CHNOSZ OBIGT | HKF / CGL / Berman species (CSV) | existing HKF and Cp methods; **Berman (1988) Cp + V**, Berman Landau (listed, not implemented) | medium | high |
| CODATA key values | ΔfH°, S°, H°(298)−H°(0) for key species | existing (reference values only) | low | medium (anchoring, checks) |
| NIST-JANAF tables (SRD 13), NIST WebBook | tabulated G, H, S, Cp vs T; **Shomate** coefficients | **Shomate** (Tier 1), **tabulated properties** method | low | medium–high (gases, high T) |
| NASA Glenn / Burcat | **NASA-7 / NASA-9** polynomials | `NASA9` exists (`new_models`); NASA-7 as Tier-1 form; enthalpy convention conversion | low | medium (gases, combustion) |
| SGTE unary / CALPHAD TDB | **G − H_SER** polynomials of elements and stoichiometric phases; magnetic term | **SGTE G polynomial** (Tier 1 `gibbs` form), **Inden–Hillert–Jarl magnetic** term, **SER ↔ formation** conversion | medium | medium–high (alloys, slags, links to CALPHAD) |
| Materials Project, OQMD, AFLOW, JARVIS (DFT) | 0 K formation energies (eV/atom), structures; AFLOW AGL Debye properties | **eV/atom → J/mol** per formula, **G(T) estimation** (Bartel descriptor or Debye), correction-scheme provenance; quality `supplemental`, origin `calculated` | medium | medium (coverage of compounds without experiments; screening) |
| MP `experimental_thermo` (MPContribs; Kubaschewski et al.) | experimental ΔfH and related values with T ranges | reference values; Shomate-like or tabulated data | low | medium |
| ThermoML / TRC | experimental raw data | out of scope for ThermoFun (fitting); adapter only (D1) | — | — |

## 2. Geochemical and evaluated standard-state databases

### 2.1 PHREEQC-format databases (phreeqc.dat, llnl.dat, Thermoddem, NEA TDB, THEREDA)

PHREEQC databases describe **reactions** (`SOLUTION_SPECIES`, `PHASES`) with `log_k` at 25 °C, `-delta_h`
(van 't Hoff), `-analytic` coefficients and `-Vm` volume parameters. NEA TDB publishes its selection as a PHREEQC
file kept up to date with its review volumes; THEREDA's main products are ready-to-use PHREEQC, GWB, ChemApp and
EQ3/6 files; a Thermoddem release (`PHREEQC_ThermoddemV1.10_15Dec2020.dat`) ships with PHREEQC.

Compatibility points:
- `-analytic A1…A6`: log K = A1 + A2·T + A3/T + A4·**log10** T + A5/T² + A6·T² *(check: PHREEQC manual)*.
  ThermoFun's `LogKAnalytic` uses **ln T** (`a3·log(T)` in C++, natural log, 06 §5.2) and has an extra `a6/√T`
  term. Mapping: a0 = A1, a1 = A2, a2 = A3, a3 = A4 / ln 10, a4 = A5, a5 = A6, a6 = 0. The adapter must convert;
  getting this wrong silently gives wrong log K at T ≠ 298 K, so a test with a known PHREEQC species is required.
- `log_k` + `-delta_h` only → `LogKExtrap2T` (van 't Hoff, constant ΔrH) *(check: ThermoFun's EK2 formula)*.
- `-Vm` (SUPCRT-like or Redlich-type volume parameters for aqueous species) — ThermoFun has no equivalent reaction-
  volume method for these; candidate new method or keep as unsupported.
- **Activity models** (Debye–Hückel `-gamma`, SIT, Pitzer in THEREDA) are not standard-state data: out of scope for
  ThermoFun (they belong to the equilibrium solver, e.g. GEMS, Reaktoro). The adapter keeps them as unknown fields.
- Reaktoro already reads PHREEQC databases; ThermoFun adding them mainly serves ThermoHub as a unified source.

### 2.2 LLNL thermo.dat (EQ3/6, GWB)

log K given at **8 temperatures** on the water saturation curve (0.01, 25, 60, 100, 150, 200, 250, 300 °C)
*(check)*. Two options:
- a **tabulated log K** method interpolating in T (legacy `logk_lagrange_interp`, id 8, exists in the enum but has
  no implementation today, 06 §5.2) — exact reproduction of the source;
- fit the 8 points to `LogKAnalytic` in the adapter — smooth, but a fit (origin `converted`).
Decision Q25: implement the tabulated method (§6, step S7.2); it also covers JANAF tables and the legacy id 8.

### 2.3 NEA TDB

Critically evaluated selected values (ΔfG°, ΔfH°, S°, Cp°, log K, reaction properties) with uncertainties, and an
electronic database / PHREEQC file. NEA uncertainties are usually given as **95 % intervals (≈ 2σ)** *(check per
volume)*; the v2 reader handles this with `{expanded: U, coverage_factor: 2}` (D9, 07 §5). This is the best test of
the uncertainty model and of reaction-defined substances (D15): NEA often selects log K and derives ΔfG°.

### 2.4 Thermoddem, ThermoChimie, Cemdata

Thermoddem (BRGM) focuses on low-temperature water/rock interactions, waste materials, cement phases and clay
minerals; ThermoChimie extended cement minerals, zeolites and clays; Cemdata18 (Empa) is already a ThermoHub /
ThermoFun database. Needed: the same reaction methods as PHREEQC (§2.1); mineral Cp functions use Maier–Kelley-type
forms that map to `CpIntegration`.

### 2.5 SUPCRTBL and Holland–Powell ds62 / ds633

Holland & Powell (2011) (ds6x) uses a **modified Tait equation of state** with a thermal pressure term (Einstein
temperature), an updated Landau treatment, and Pitzer–Sterner (1995) for gases; versions 6.2 and 6.33 are in wide
use with THERMOCALC. SUPCRTBL takes most mineral end-members from Holland & Powell (2011) with HKF aqueous species.

ThermoFun: `HP98Min` (Murnaghan, HP1998), `HP98Landau`, `SternerPitzer`, and `HP11Tait` (modified Tait EoS with
thermal pressure, from the `new_models` branch, decision Q29). Adding the HP2011 Landau model (Tier 2) lets ThermoFun
use ds62/ds633 and SUPCRTBL minerals directly. Adapter: the ds62 text file (`tc-ds62.txt`) → v2.

### 2.6 DEW (Deep Earth Water)

Extends revised HKF to 60 kbar and 1200 °C (Sverjensky, Harrison & Azzolini 2014), with water density from
Zhang & Duan (2005) and the dielectric constant of Sverjensky et al. (2014); CHNOSZ implements it from the DEW
spreadsheet's VBA code. ThermoFun already has `WaterZhangDuan05` and `Sverjensky14Diel`; a DEW dataset may be
mostly **data** (HKF parameters fitted for DEW, a water record using these two methods). To check: the DEW water
Gibbs energy function (`calculateGibbsOfWater`, valid 100–1000 °C, P ≥ 1 kbar) vs ThermoFun's water G, and DEW's
ω(P,T) handling for charged species. Validity ranges from the DEW code should become method `applicability`.

### 2.7 CHNOSZ OBIGT

CSV with model column `HKF` / `CGL` / `Berman` (see 01 §3). HKF and CGL map to existing methods
(`HKFReaktoro`, `CpIntegration` + `ConMolVol`); **Berman (1988) minerals** need Berman's Cp polynomial
(k0 + k1 T^-0.5 + k2 T^-2 + k3 T^-3 — expressible with `CpIntegration` coefficients *(check mapping)*), Berman's
volume function (`Berman88Vol`, exists), and Berman lambda/disorder terms (`Berman88Landau`, listed, not
implemented).

### 2.8 CODATA key values

Internationally agreed ΔfH°(298.15 K), S°(298.15 K) and H°(298.15) − H°(0) for key substances (Cox, Wagman &
Medvedev 1989), standard pressure 1 bar. Use: anchor values for elements and key species, and a validation set
(ThermoFun reference values vs CODATA). No new method.

## 3. Gas-phase, high-temperature and CALPHAD sources

### 3.1 NIST-JANAF (SRD 13) and the NIST WebBook

JANAF 4th edition: tables for ~1800 substances, 0–6000 K, SI, distributed online as NIST SRD 13 (HTML/text). The
WebBook tabulates **Shomate** coefficients derived from JANAF, per temperature range:
Cp = A + B t + C t² + D t³ + E/t², t = T/1000 K; H − H298 = A t + B t²/2 + C t³/3 + D t⁴/4 − E/t + F − H;
S = A ln t + B t + C t²/2 + D t³/3 − E/(2t²) + G (H in kJ/mol).

Needed: a **Shomate** method — a Tier-1 `cp` source form with t = T/1000 (06 §10), piecewise in T; or tabulated
JANAF values with the tabulated method (§4.3). Licensing of NIST SRD data must be checked before redistribution
in `thermohub-data` *(check)*.

### 3.2 NASA Glenn / Burcat (NASA-7, NASA-9)

NASA-7: Cp/R = a1 + a2 T + a3 T² + a4 T³ + a5 T⁴, with a6, a7 as integration constants for H/RT and S/R, usually
two ranges (200–1000–6000 K); NASA-9 adds terms and covers up to 20 000 K. Burcat's database is a widely used
NASA-7 source; Cantera reads both forms. Needed: Tier-1 source forms `nasa7`, `nasa9` (06 §10). **Convention:**
NASA H includes the enthalpy of formation at 298.15 K (assigned enthalpy convention) — conversion to ThermoFun's
apparent formation properties needs the element reference data (§5.1).

### 3.3 SGTE unary and CALPHAD (TDB)

SGTE data for 78 pure elements (Dinsdale 1991) and SGTE substance data give **G − H_SER(298.15 K)** as
G = a + b T + c T ln T + d T² + e T³ + f/T (+ g T⁷ or h T⁻⁹ above/below the melting point) *(check exact terms)*,
piecewise in T, with a separate magnetic contribution (Inden–Hillert–Jarl:
G_mag = R T ln(β + 1) f(τ), τ = T/T_C, β the mean magnetic moment, f piecewise; antiferromagnetic and structure
factors). The TDB format is the de facto exchange format (pycalphad reads it); an XML successor has been proposed.

Needed:
- Tier-1 `gibbs` source form (the SGTE polynomial is already a G function) — no integration required;
- **Inden–Hillert–Jarl magnetic term** as a Tier-1 `gibbs_increment` with piecewise f(τ) (parameters T_C, β,
  structure factor p);
- **SER ↔ formation conversion** (§5.1);
- a TDB adapter for **stoichiometric phases only** (solution models are out of scope).

## 4. Computational materials databases

### 4.1 Materials Project

- Provides DFT **0 K** total and formation energies (eV/atom) for >150 000 inorganic compounds, with energy
  corrections and a GGA/GGA+U/r2SCAN mixing scheme for phase-diagram construction (Kingsbury et al. 2022); data
  CC-BY 4.0 *(check current terms)*; API via `mp-api`/pymatgen.
- Formation energies depend on the correction scheme and on the chemical system used (forum reports: values shown
  are computed within the material's own system) — the adapter must record the **thermo type and correction
  scheme** as provenance (`origin: calculated`, `calculation_path`).
- No temperature dependence. Options for G(T): the **Bartel et al. (2018) descriptor** — a closed-form G(T) increment
  from mass, density and T, ~50 meV/atom error over 300–1800 K, not suited to polymorph or magnetic transitions
  (pymatgen implements it) → a Tier-1 `gibbs_increment` method with quality `supplemental`; or Debye/phonon data
  (§4.2).
- **Experimental thermochemistry** (from Kubaschewski, Alcock & Spencer, *Materials Thermochemistry*) is available as
  the MPContribs project `experimental_thermo` — reference values with T ranges, citable.
- Value for ThermoFun/ThermoHub: coverage of compounds without experimental data, screening, and cross-checks of
  evaluated values; never mixed silently with evaluated data (quality flags, separate `sourcetdb`).

### 4.2 OQMD, AFLOW, JARVIS

- **OQMD**: ~300 000 DFT total energies and formation energies, bulk download, CC-BY 4.0, published comparison of DFT
  vs experimental formation energies — same treatment as MP.
- **AFLOW AGL** (quasi-harmonic Debye model from E(V) curves): Debye temperature, Grüneisen parameter, thermal
  expansion, bulk modulus for thousands of materials via the AFLOW REST API. A **Debye model method** (Tier 2:
  Debye function integral, parameters θ_D, Grüneisen γ, V0, K0) could turn these into Cp(T), S(T), G(T). Accuracy of
  the Grüneisen parameter is reported as poor, so use with care.
- **JARVIS-DFT**: formation and total energies, public; terms *(check)*.
- Most of these databases offer an **OPTIMADE** API (standard queries for structures and some properties);
  thermodynamic fields are provider-specific, so adapters use OPTIMADE for discovery and provider APIs for energies.

## 5. Compatibility issues ThermoFun and the schema must handle

### 5.1 Energy reference conventions (implemented by the conventions layer, 14)

| Convention | Used by | Meaning | Conversion to ThermoFun's apparent formation G |
|---|---|---|---|
| Apparent formation (Benson–Helgeson) | SUPCRT, slop, aq17, PHREEQC (via log K) | ΔfG°(Tr) + G(T) − G(Tr) | none (today's default) |
| Berman–Brown | Berman, HP | G = ΔfH°(Tr) − T S° + … | exists (`EngineConventions.apparent_properties`) |
| SER (CALPHAD, SGTE) | TDB, SGTE | G − Σ H_SER(elements, 298.15 K) | needs element SER data: subtract Σν_i [H_SER,i − T S_i] terms consistently |
| Assigned enthalpy (NASA, JANAF H − H298 + ΔfH) | NASA-7/9 (incl. ThermoFun `NASA9`), JANAF, Burcat | H includes ΔfH(298) | needs element reference H and S |
| Gibbs–Helmholtz from ΔfG, ΔfH | Extended UNIQUAC (ThermoFun `ExtendedUNIQUAC`) | G(T) = Gr·T/Tr + Hr(1 − T/Tr) + Cp terms | entropy term uses ΔfS, not S°: offset (T − Tr)·Σ S(elements) vs Benson–Helgeson; converts with element entropies |
| DFT 0 K formation energy | MP, OQMD, AFLOW, JARVIS | ΔfE at 0 K, per atom, with corrections | per-formula scaling, eV → J/mol, + G(T) model; treat as ΔfH(298) estimate |

Decision Q24: add an `energy_reference` field to the standard-state convention of a record or dataset
(`apparent_formation | berman_brown | ser | assigned_enthalpy | dft_formation_0K`), and give Element records the
reference data needed for conversions (SER H and S at 298.15 K, already partly there as `standard_entropy`).
ThermoFun converts at load to its internal convention, as it already does for Berman–Brown.

### 5.2 Units and amounts

- **eV/atom** (DFT databases): needs the number of atoms per formula unit (from the formula, ChemicalFun) and
  eV → J/mol (× 96 485.33 J/(mol·eV)). The converter has no `eV` atom today; add `eV` and handle "per atom" in the
  adapter, not in the unit system (per-atom is an amount basis, not a unit).
- **kJ/mol and kcal/mol**, **cal = thermochemical** (D14) — covered.
- **Standard pressure 1 atm vs 1 bar**: older tables (NBS, some JANAF-derived data) use 1 atm; gas entropies and
  Gibbs energies differ by R ln(1.01325) ≈ 0.109 J/(mol·K). Decision Q26: a `standard_pressure` field in the
  convention and a conversion at load for gases.

### 5.3 Data quality and mixing

DFT-derived and estimated data must carry `origin: calculated|estimated` and quality `supplemental` (D17), live in
their own `sourcetdb`, and never replace evaluated data without the user choosing a ThermoDataSet that includes them.

### 5.4 Licences

CC-BY 4.0 (MP, OQMD) is compatible with `thermohub-data` (CC-BY 4.0, D52) with attribution. NIST SRD, NEA TDB,
Thermoddem, SGTE, Burcat, ds62 have their own terms *(check each)*; adapters can be published regardless, but data
redistribution needs per-source checks.

## 6. New methods ThermoFun could cover (most as Tier-1 data, thanks to Q15)

| Method | Tier | Form (06 §10) | Enables |
|---|---|---|---|
| `Shomate` | 1 | new source form `shomate` (Cp in t = T/1000, piecewise) | NIST WebBook / JANAF species |
| `NASA7` | 1 | new source form `nasa7` (`NASA9` exists as a coded method, `new_models`) | NASA Glenn, Burcat, Cantera data |
| `SGTEPolynomial` | 1 | `gibbs` (piecewise) + SER convention | SGTE unary/substance, CALPHAD stoichiometric phases |
| `MagneticIHJ` | 1 | `gibbs_increment`, piecewise f(τ) | SGTE/CALPHAD magnetic elements and compounds |
| `BermanCp` mapping | 1 / existing | `cp` | OBIGT/Berman minerals |
| `BartelGibbs` | 1 | `gibbs_increment` from mass, V/atom | approximate G(T) for DFT compounds |
| `TPTableInterpolation` (exists, `new_models`); consistent-spline option if needed | 2 | interpolation of G, H, V, Cp on a T–P grid | JANAF tables, any tabulated source |
| `TabulatedLogK` | 2 | interpolation of log K on a T grid (legacy id 8 `logk_lagrange_interp`) | LLNL thermo.dat, EQ3/6, GWB |
| `HP11Landau` (`HP11Tait` exists, `new_models`) | 2 | C++ | Holland–Powell ds62/ds633, SUPCRTBL |
| `ExtendedUNIQUAC` (exists, `new_models`) | 2 | C++ | Extended UNIQUAC (Thomsen) data, as in Reaktoro |
| `Berman88Landau` | 2 | C++ (listed, not implemented) | Berman minerals with lambda transitions |
| `DebyeQHA` | 2 | C++ (Debye integral) | AFLOW AGL, phonon-derived data |
| PHREEQC `-Vm` aqueous volume | 2 | C++ | PHREEQC P dependence of log K |

The Tier-1 ones need no C++ code once S5.3–S5.4 exist: a contributor writes the source form, the tool builds it.
New **source forms** (`shomate`, `nasa7`, `nasa9`) are small additions to the authoring tool (06 §10.2).

## 7. Proposed priorities

1. **PHREEQC-format reaction data** (analytic log K with the log10 conversion, van 't Hoff) and **TabulatedLogK** —
   largest user base; also implements the dormant legacy id 8.
2. **HP11Landau** (Tait exists) — completes the Holland–Powell family; opens ds62/ds633 and
   SUPCRTBL.
3. **Shomate, NASA-7, SGTE polynomial, magnetic IHJ** as Tier-1 forms (NASA-9 exists) — cheap after Q15's tooling; opens gas and
   CALPHAD data.
4. **Energy reference conventions** (SER, assigned enthalpy) and **standard pressure** conversion — needed by 3.
5. **DFT databases** (MP, OQMD) with eV/atom handling, provenance of correction schemes and Bartel G(T) —
   lower accuracy, high coverage; mostly adapter work in `thermohub-tools`.
6. **DEW** — check what is missing beyond existing water methods; likely a dataset rather than code.

## 8. Decisions

Q23–Q28 (target order, energy reference conventions, tabulated methods, standard pressure, DFT data, activity
models out of scope) are recorded in [12-decisions.md](12-decisions.md) as **provisional** decisions, taken at the
recommendations of this file; the work is planned in Phase 7 of [11-implementation-steps.md](11-implementation-steps.md).

## Sources

- Materials Project mixing scheme: [MP documentation](https://docs.materialsproject.org/methodology/materials-methodology/thermodynamic-stability/thermodynamic-stability/gga-gga+u-r2scan-mixing), [Kingsbury et al. 2022](https://kavli.berkeley.edu/publications/flexible-and-scalable-scheme-mixing-computed-formation-energies-different-levels-theory), forum threads on [corrections](https://matsci.org/t/how-to-apply-corrections-to-scan-energy-calculations/64371) and [r2SCAN hulls](https://matsci.org/t/correcting-r2scan-data-to-find-ehull-with-respect-to-gga-gga-u-r2scan-phase-diagram/65758); [OMat24](https://arxiv.org/pdf/2410.12771).
- MP experimental data: [Shomate/ThermoData thread](https://matsci.org/t/query-shomate-polynomial-using-pymatgen/1237), [MPContribs](https://docs.materialsproject.org/services/mpcontribs), [MPContribs paper](https://arxiv.org/pdf/1510.05727).
- Bartel G(T) descriptor: [Nature Communications 2018 (PMC)](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6177451/), [arXiv 1805.08155](https://arxiv.org/pdf/1805.08155).
- AFLOW AGL: [Toher et al. 2014](https://arxiv.org/pdf/1407.7789), [GIBBS + Elastic libraries](https://arxiv.org/pdf/1611.05714), [automated properties](https://arxiv.org/pdf/1805.05309).
- OQMD: [Kirklin et al. (OQMD accuracy)](https://oa.tib.eu/renate/items/845c9719-3ced-4492-b2af-81264f41d570); JARVIS mention in [CrysGNN](https://arxiv.org/pdf/2301.05852).
- NIST-JANAF: [data.gov SRD 13](https://catalog.data.gov/dataset/nist-janaf-thermochemical-tables-srd-13), [Thermodex record](https://thermodex.lib.utexas.edu/record/nist-janaf-thermochemical-tables-4th-ed/).
- NASA/Burcat: [assessment of NASA-7 databases](https://arxiv.org/pdf/2108.05444), [MFiX thermochemical properties](https://mfix.netl.doe.gov/doc/mfix/24.1/html/reference/thermochemical_properties.html).
- SGTE/CALPHAD: [Computational thermodynamics (Wikipedia)](https://en.wikipedia.org/wiki/Computational_thermodynamics), [SGTE data for pure elements](https://scholarhub.vn/publication/:slug/e15da9a4-8c50-4725-865f-44330f0fee01), [XML successor of TDB](https://air.uniud.it/bitstream/11390/1311064/1/1-s2.0-S0364591625000525-main.pdf); magnetic model: [Thermo-Calc help](https://help.thermocalc.com/2025b/content/data_optimization/magnetic_ordering_about.htm), [arXiv 2607.12388](https://arxiv.org/pdf/2607.12388).
- Geochemical databases: [NEA TDB electronic database](https://oecd-nea.org/jcms/pl_37223/the-new-electronic-database-of-the-thermochemical-database-project-tdb), [llnl.dat](https://search.r-project.org/CRAN/refmans/phreeqc/html/llnl.dat.html), [Thermoddem in PHREEQC](https://search.r-project.org/CRAN/refmans/phreeqc/html/PHREEQC_ThermoddemV1.10_15Dec2020.dat.html), [Thermoddem paper](https://hal-univ-tlse3.archives-ouvertes.fr/hal-00846739), [ThermoChimie cement/clay](https://inis.iaea.org/records/48wjs-9ze36), [Cemdata18](https://www.empa.ch/documents/55996/86308/Lothenbach-2019-Cemdata18/3a7f15d5-62f0-41b1-a038-746b3f588acb), [database comparison 2018](https://link.springer.com/article/10.1186/s40517-018-0106-3), [GWB datasets](https://community.gwb.com/thermo.php).
- Holland–Powell / SUPCRTBL: [HP2011 dataset paper](https://pages.uoregon.edu/palandri/gastherm/!data/!refs/!Holland-Powell/JMetGeol_2011_HollandPowell_dataset.pdf), [HPx-eos implementation](https://hpxeosandthermocalc.org/the-hpx-eos/hpx-eos-implementation/), [THERMOCALC dataset extraction](https://hpxeosandthermocalc.org/the-thermocalc-software/thermocalc-calculation-facilities/thermocalc/extracting-dataset-information/), [pygcc species EoS](https://pygcc.readthedocs.io/en/latest/autoapi/pygcc/species_eos/index.html).
- DEW: [CHNOSZ DEW](https://search.r-project.org/CRAN/refmans/CHNOSZ/html/DEW.html), [pyCHNOSZ DEW](https://worm-portal.asu.edu/pyCHNOSZ-docs/models/dew.html).
- CODATA: [CODATA key values](https://www.codata.info/resources/databases/key1.html).
- CHNOSZ OBIGT: [custom data vignette](https://cran.r-project.org/web/packages/CHNOSZ/vignettes/custom_data.html).
