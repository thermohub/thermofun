// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "LegacyMethodMap.h"

namespace ThermoFun::v2::legacy {

namespace {

using P = std::vector<LegacyParameter>;

auto cpCoefficients() -> P
{
    const char* k = "m_heat_capacity_ft_coeffs";
    return {{k, 0, "a0", "J/(mol*K)"}, {k, 1, "a1", "J/(mol*K^2)"}, {k, 2, "a2", "(J*K)/mol"}, {k, 3, "a3", "J/(mol*K^0.5)"},
            {k, 4, "a4", "J/(mol*K^3)"}, {k, 5, "a5", "J/(mol*K^4)"}, {k, 6, "a6", "J/(mol*K^5)"}, {k, 7, "a7", "(J*K^2)/mol"},
            {k, 8, "a8", "J/mol"}, {k, 9, "a9", "J/(mol*K^1.5)"}};
}

auto hkf() -> P
{
    const char* k = "eos_hkf_coeffs";
    return {{k, 0, "a1", "cal/(mol*bar)"}, {k, 1, "a2", "cal/mol"}, {k, 2, "a3", "(cal*K)/(mol*bar)"}, {k, 3, "a4", "(cal*K)/mol"},
            {k, 4, "c1", "cal/(mol*K)"}, {k, 5, "c2", "(cal*K)/mol"}, {k, 6, "omega", "cal/mol"}};
}

auto volume(const char* k) -> P
{
    return {{k, 0, "a1", "1/K"}, {k, 1, "a2", "1/K^2"}, {k, 2, "a3", "1/K^3"}, {k, 3, "b1", "1/bar"}, {k, 4, "b2", "1/bar^2"}};
}

auto critical() -> P
{
    const char* k = "eos_gas_crit_props";
    return {{k, 0, "Tc", "K"}, {k, 1, "Pc", "bar"}, {k, 2, "omega", "1"}};
}

auto cubic() -> P
{
    auto out = critical();
    out.push_back({"eos_gas_crit_props", 3, "N", "1"});
    return out;
}

auto interaction() -> P
{
    auto out = critical();
    out.push_back({"eos_gas_crit_props", 3, "k1", "1"});
    out.push_back({"eos_gas_crit_props", 4, "k2", "1"});
    out.push_back({"eos_gas_crit_props", 5, "k3", "1"});
    return out;
}

// the array eos_gas_crit_props is read first: eos_churakov_gottschalk_coeffs replaces it if the entry has both
auto churakovGottschalk() -> P
{
    P out;
    const char* names[] = {"sigma0", "eps0", "mu0", "pol0", "sigma1", "sigma2", "eps1", "eps2", "mu1", "mu2", "pol1", "pol2"};
    for (const char* k : {"eos_gas_crit_props", "eos_churakov_gottschalk_coeffs"})
        for (int i = 0; i < 12; ++i)
            out.push_back({k, i, names[i], "1"});
    return out;
}

// The coefficients of log K(T) and of dCp(T) can be given with any method of log K
auto logK() -> P
{
    const char* k = "logk_ft_coeffs";
    const char* c = "dr_heat_capacity_ft_coeffs";
    return {{k, 0, "a0", "1"}, {k, 1, "a1", "1/K"}, {k, 2, "a2", "K"}, {k, 3, "a3", "1"}, {k, 4, "a4", "K^2"}, {k, 5, "a5", "1/K^2"},
            {k, 6, "a6", "K^0.5"},
            {c, 0, "a", "J/(mol*K)"}, {c, 1, "b", "J/(mol*K^2)"}, {c, 2, "c", "(J*K)/mol"}, {c, 3, "d", "J/(mol*K^0.5)"}, {c, 4, "e", "J/(mol*K^3)"}};
}

auto find(const std::vector<LegacyMethod>& methods, int id) -> const LegacyMethod*
{
    for (const auto& m : methods)
        if (m.id == id)
            return &m;
    return nullptr;
}

} // namespace

auto legacySubstanceMethods() -> const std::vector<LegacyMethod>&
{
    static const std::vector<LegacyMethod> methods = {
        {0, "cp_ft_equation", "CpPolynomial", "", 100, cpCoefficients()},
        {1, "cp_ft_equation_saxena86", "CpFeiSaxena86", "", 203, {}},
        {2, "solute_hkf88_gems", "HKF88", "gems", 101, hkf()},
        {3, "solute_hkf88_reaktoro", "HKF88", "reaktoro", 119, hkf()},
        {4, "solute_aknifiev_diamond03", "AkinfievDiamond03", "", 320,
         {{"eos_akinfiev_diamond_coeffs", 0, "xi", "1"}, {"eos_akinfiev_diamond_coeffs", 1, "a", "(cm^3)/g"},
          {"eos_akinfiev_diamond_coeffs", 2, "b", "(cm^3*K^0.5)/g"}}},
        {5, "landau_holland_powell98", "LandauHP98", "", 201,
         {{"m_landau_phase_trans_props", 0, "Tc0", "degC"}, {"m_landau_phase_trans_props", 1, "Smax", "J/(mol*K)"},
          {"m_landau_phase_trans_props", 2, "Vmax", "J/(mol*bar)"}}},
        {6, "landau_berman88", "LambdaBerman88", "", 202, {}},
        {8, "fug_critical_param", "", "", 311, critical()},   // a flag: see the water vapour rule of the reader
        {9, "fluid_prsv", "PRSV86", "", 312, interaction()},
        {10, "fluid_churakov_gottschalk", "ChurakovGottschalk03", "", 313, churakovGottschalk()},
        {11, "fluid_soave_redlich_kwong", "SoaveRedlichKwong72", "", 314, cubic()},
        {12, "fluid_sterner_pitzer", "SternerPitzer94", "", 315, {{"eos_gas_crit_props", 0, "Tc", "K"}, {"eos_gas_crit_props", 1, "Pc", "bar"}}},
        {13, "fluid_peng_robinson78", "PengRobinson78", "", 316, cubic()},
        {14, "fluid_comp_redlich_kwong_hp91", "CorkHP91", "", 317, interaction()},
        {25, "water_diel_jnort91_reaktoro", "JohnsonNorton91", "reaktoro", 117, {}},
        {26, "water_diel_jnort91_gems", "JohnsonNorton91", "gems", 118, {}},
        {27, "water_diel_sverj14", "SverjenskyEtAl14", "", 120, {}},
        {28, "water_diel_fern97", "FernandezEtAl97", "", 121, {}},
        {29, "water_eos_hgk84_lvs83_gems", "WaterHGK84", "gems", 206, {}},
        {30, "water_eos_iapws95_gems", "WaterIAPWS95", "reaktoro", 207, {}},  // there is no GEMS code: LEG-MTH-003
        {31, "water_eos_hgk84_reaktoro", "WaterHGK84", "reaktoro", 221, {}},
        {32, "water_eos_iapws95_reaktoro", "WaterIAPWS95", "reaktoro", 222, {}},
        {33, "water_pvt_zhang_duan05", "WaterZhangDuan05", "", 223, {}},
        {34, "mv_constant", "ConstantVolume", "", 305, {}},
        {35, "mv_equation_dorogokupets88", "VolumeDorogokupets88", "", 306, volume("m_volume_fpt_coeffs")},
        {36, "mv_equation_berman88", "VolumeBerman88", "", 307, volume("m_volume_fpt_coeffs")},
        {37, "mv_eos_birch_murnaghan_gott97", "BirchMurnaghan47", "", 308,
         {{"eos_birch_murnaghan_coeffs", 0, "a1", "1/K"}, {"eos_birch_murnaghan_coeffs", 1, "a2", "1/K^2"},
          {"eos_birch_murnaghan_coeffs", 2, "a3", "K"}, {"eos_birch_murnaghan_coeffs", 5, "kt00", "MPa"},
          {"eos_birch_murnaghan_coeffs", 6, "dkdt", "MPa/K"}, {"eos_birch_murnaghan_coeffs", 7, "kp", "1"},
          {"eos_birch_murnaghan_coeffs", 8, "kpp", "1/MPa"}}},
        {38, "mv_eos_murnaghan_hp98", "MurnaghanHP98", "", 309, {}},
        {39, "mv_eos_tait_hp11", "TaitHP11", "", 310,
         {{"eos_tait_hp11_coeffs", 0, "kappa0p", "1"}, {"eos_tait_hp11_coeffs", 1, "kappa0pp", "1/kbar"}, {"eos_tait_hp11_coeffs", 2, "n_atoms", "1"}}},
        {40, "mv_pvnrt", "IdealGas", "", 300, {}},
        {41, "solute_holland_powell98", "SoluteHP98", "", 122, {{"solute_holland_powell98_coeff", 0, "b", "kJ/(mol*K^2)"}}},
        {42, "solute_anderson91", "AndersonEtAl91", "", 123, {}},
        {43, "standard_entropy_cp_integration", "ConstantCp", "", 200, {}},
        {44, "cp_ft_equation_uniquac", "CpExtendedUNIQUAC", "", 225,
         {{"eos_extended_uniquac_coeffs", 0, "a", "J/(mol*K)"}, {"eos_extended_uniquac_coeffs", 1, "b", "J/(mol*K^2)"},
          {"eos_extended_uniquac_coeffs", 2, "c", "J/mol"}, {"eos_extended_uniquac_coeffs", 3, "alpha", "1/bar"},
          {"eos_extended_uniquac_coeffs", 4, "beta", "1/bar^2"}, {"eos_extended_uniquac_coeffs", 5, "Theta", "K"}}},
        {45, "cp_nasa_polynomials", "NASA9", "", 226,
         {{"nasa_polynomial_coeffs", 0, "a1", "1"}, {"nasa_polynomial_coeffs", 1, "a2", "1"}, {"nasa_polynomial_coeffs", 2, "a3", "1"},
          {"nasa_polynomial_coeffs", 3, "a4", "1"}, {"nasa_polynomial_coeffs", 4, "a5", "1"}, {"nasa_polynomial_coeffs", 5, "a6", "1"},
          {"nasa_polynomial_coeffs", 6, "a7", "1"}, {"nasa_polynomial_coeffs", 7, "b1", "K"}, {"nasa_polynomial_coeffs", 8, "b2", "1"}}},
        {46, "standard_thermo_interpolation", "TPTable", "", 227, {}},
    };
    return methods;
}

auto legacyReactionMethods() -> const std::vector<LegacyMethod>&
{
    static const std::vector<LegacyMethod> methods = {
        {0, "logk_fpt_function", "LogKPolynomial", "", 102, logK()},
        {1, "adsor_ion_exchange", "", "", 0, {}},
        {2, "iso_compounds_grichuk88", "", "", 0, {}},
        {3, "logk_nordstrom_munoz88", "LogKNordstromMunoz88", "", 208, logK()},
        {4, "logk_1_term_extrap0", "LogKConstant", "", 209, logK()},
        {5, "logk_1_term_extrap1", "LogKZeroEntropy", "", 210, logK()},
        {6, "logk_2_term_extrap", "LogKVantHoff", "", 211, logK()},
        {7, "logk_3_term_extrap", "LogKConstantCp", "", 212, logK()},
        {8, "logk_lagrange_interp", "LogKTable", "", 213, {}},
        {9, "logk_marshall_frank78", "MarshallFranck81", "", 214,
         {{"dr_marshall_franck_coeffs", 0, "a", "1"}, {"dr_marshall_franck_coeffs", 1, "b", "K"}, {"dr_marshall_franck_coeffs", 2, "c", "K^2"},
          {"dr_marshall_franck_coeffs", 3, "d", "K^3"}, {"dr_marshall_franck_coeffs", 4, "e", "1"}, {"dr_marshall_franck_coeffs", 5, "f", "K"},
          {"dr_marshall_franck_coeffs", 6, "g", "K^2"}}},
        {10, "solute_eos_ryzhenko_gems", "RyzhenkoBryzgalin", "", 216,
         {{"dr_ryzhenko_coeffs", 0, "pK298", "1"}, {"dr_ryzhenko_coeffs", 1, "A", "1"}, {"dr_ryzhenko_coeffs", 2, "B", "1"}}},
        {11, "dr_heat_capacity_ft", "ReactionCpPolynomial", "", 100,
         {{"dr_heat_capacity_ft_coeffs", 0, "a", "J/(mol*K)"}, {"dr_heat_capacity_ft_coeffs", 1, "b", "J/(mol*K^2)"},
          {"dr_heat_capacity_ft_coeffs", 2, "c", "(J*K)/mol"}, {"dr_heat_capacity_ft_coeffs", 3, "d", "J/(mol*K^0.5)"},
          {"dr_heat_capacity_ft_coeffs", 4, "e", "J/(mol*K^3)"}}},
        {12, "dr_volume_fpt", "ReactionVolumePolynomial", "", 306, volume("dr_volume_fpt_coeffs")},
        {13, "dr_volume_constant", "ReactionConstantVolume", "", 305, {}},
        {14, "logk_dolejs_manning10", "DolejsManning10", "", 224,
         {{"dr_dolejs_manning10_coeffs", 0, "a", "kJ/mol"}, {"dr_dolejs_manning10_coeffs", 1, "b", "J/(mol*K)"},
          {"dr_dolejs_manning10_coeffs", 2, "c", "J/(mol*K)"}, {"dr_dolejs_manning10_coeffs", 3, "d", "J/(mol*K^2)"},
          {"dr_dolejs_manning10_coeffs", 4, "e", "J/(mol*K)"}}},
    };
    return methods;
}

auto legacySubstanceMethod(int id) -> const LegacyMethod* { return find(legacySubstanceMethods(), id); }

auto legacyReactionMethod(int id) -> const LegacyMethod* { return find(legacyReactionMethods(), id); }

auto legacyCoefficientKeys() -> const std::set<std::string>&
{
    static const std::set<std::string> keys = [] {
        std::set<std::string> k;
        for (const auto* list : {&legacySubstanceMethods(), &legacyReactionMethods()})
            for (const auto& m : *list)
                for (const auto& p : m.parameters)
                    k.insert(p.key);
        return k;
    }();
    return keys;
}

} // namespace ThermoFun::v2::legacy
