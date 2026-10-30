# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Write the synthetic legacy fixtures in pytests/data/legacy-synthetic/.

Each file holds the records that one method needs, for methods that no bundled database uses.
Coefficients come from tests/gems_export/gems_export_data.json, from the bundled databases, or from
the cited papers. The fixtures are small inputs for the golden suite, not calibrated data.
Run it once; the files are committed.
"""

import copy
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / "pytests" / "data" / "legacy-synthetic"
GEMS_EXPORT = json.loads((ROOT / "tests" / "gems_export" / "gems_export_data.json").read_text())
GEMS_SUBSTANCES = {s["symbol"]: s for s in GEMS_EXPORT["substances"]}


def method(code, name):
    return {"method": {str(code): name}}


def values(vals, units):
    return {"values": vals, "units": units}


def water(extra=None):
    """Solvent H2O@ with the Reaktoro water models; extra: method entries added at the end."""
    methods = [method(31, "water_eos_hgk84_reaktoro"), method(25, "water_diel_jnort91_reaktoro")]
    return {
        "name": "Water HGK", "symbol": "H2O@", "formula": "H2O@", "formula_charge": 0,
        "aggregate_state": {"4": "AS_AQUEOUS"}, "class_": {"3": "SC_AQSOLVENT"},
        "Tst": 298.15, "Pst": 100000, "TPMethods": methods + (extra or []),
        "sm_heat_capacity_p": values([75.360527038574], ["J/(mol*K)"]),
        "sm_gibbs_energy": values([-237183], ["J/mol"]),
        "sm_enthalpy": values([-285881], ["J/mol"]),
        "sm_entropy_abs": values([69.922996520996], ["J/(mol*K)"]),
        "sm_volume": values([1.8068397045136], ["J/bar"]),
        "_label": "substance",
        "_comment": "Solvent as in pytests/Substances/Solute/test-hkf-thermofun.json, with the Reaktoro water models",
    }


def solute():
    """Al+3 with the HKF model (Slop98 values of pytests/Substances/Solute/test-hkf-thermofun.json)."""
    return {
        "name": "Al+3 ion", "symbol": "Al+3", "formula": "Al+3", "formula_charge": 3,
        "aggregate_state": {"4": "AS_AQUEOUS"}, "class_": {"2": "SC_AQSOLUTE"}, "Tst": 298.15, "Pst": 100000,
        "TPMethods": [{
            "method": {"3": "solute_hkf88_reaktoro"},
            "eos_hkf_coeffs": {"values": [-0.33802, -1700.71, 14.5185, -20758, 10.7, -80600, 275300, 0],
                               "units": ["cal/(mol*bar)", "cal/mol", "(cal*K)/mol", "cal/(mol*K)", "(cal*K)/mol",
                                         "cal/mol"],
                               "names": ["a1", "a2", "a3", "a4", "c1", "c2", "wref"]}}],
        "sm_heat_capacity_p": values([-128.696], ["J/(mol*K)"]),
        "sm_gibbs_energy": values([-483708], ["J/mol"]),
        "sm_enthalpy": values([-530673], ["J/mol"]),
        "sm_entropy_abs": values([-325.097], ["J/(mol*K)"]),
        "sm_volume": values([-4.5243], ["J/bar"]),
        "_label": "substance",
    }


def from_gems_export(symbol, new_symbol, extra_methods):
    """A record of the GEMS export test data; its TPMethods are replaced by its first (Cp) entry and extra_methods."""
    rec = copy.deepcopy(GEMS_SUBSTANCES[symbol])
    cp_entry = [m for m in rec["TPMethods"] if "0" in m["method"]]
    rec["TPMethods"] = cp_entry + extra_methods
    rec["symbol"] = new_symbol
    rec["name"] = new_symbol
    rec["_comment"] = "Cp and reference values of %s in tests/gems_export/gems_export_data.json" % symbol
    rec.pop("datasources", None)
    return rec


def solid(new_symbol, extra_methods):
    return from_gems_export("Magnesite", new_symbol, extra_methods)


def gas(new_symbol, extra_methods):
    return from_gems_export("CO", new_symbol, extra_methods)


CRIT = {"eos_gas_crit_props": {
    "values": [132.91000366211, 34.959999084473, 0.048300001770258, 0.042789999395609, 0, 0, 0],
    "units": ["K", "bar", "", ""], "names": ["Tcr", "Pcr", "W", "k1"]}}
VOLUME_FPT = {"m_volume_fpt_coeffs": {
    "values": [-8.900000239009387e-07, 2.2119998974862165e-12, 0, 1.8435999663779512e-05, 4.1596798894261156e-08]}}
BM = {"eos_birch_murnaghan_coeffs": {
    "values": [3.73e-05, 0, 0, 0, 0, 121951.2, 0, 4.0, 0],
    "_comment": "a1..a5 [1/K...], kt00 [MPa], dkdt [MPa/K], kp, kpp (0: computed). Magnesite: a1 = alpha = 37.30e-6 1/K "
                "and kt00 = 1/beta, beta = 8.20e-6 1/MPa, from Gottschalk (1997) Table 2; kp = 4 (second-order "
                "Birch-Murnaghan) and dkdt = 0 are not from the paper, which gives no Birch-Murnaghan parameters"}}


def za_zp():
    """Two solids with Cp and reference values, the species of the reaction fixtures."""
    out = []
    for sym, g, h in [("ZA", -100000.0, -120000.0), ("ZP", -90000.0, -110000.0)]:
        out.append({
            "name": sym, "symbol": sym, "formula": "Mg", "formula_charge": 0, "mass_per_mole": values([24.3], ["g/mol"]),
            "aggregate_state": {"3": "AS_CRYSTAL"}, "class_": {"0": "SC_COMPONENT"}, "Tst": 298.15, "Pst": 100000,
            "sm_gibbs_energy": values([g], ["J/mol"]), "sm_enthalpy": values([h], ["J/mol"]),
            "sm_entropy_abs": values([50.0], ["J/(mol*K)"]), "sm_heat_capacity_p": values([75.0], ["J/(mol*K)"]),
            "sm_volume": values([0.0], ["J/bar"]), "TPMethods": [method(0, "cp_ft_equation")],
            "_comment": "Same values as ZA and ZP in tests/gems_export/gems_export_data.json"})
    return out


def reaction(symbol, methods):
    """Reaction ZA = ZP with reference values of tests/gems_export; methods: TPMethods entries."""
    return {
        "symbol": symbol, "equation": "ZA = ZP", "Tst": 298.15, "Pst": 100000,
        "logKr": {"values": [-1.7519142195881732], "errors": [0]},
        "drsm_gibbs_energy": values([10000.0], ["J/mol"]), "drsm_enthalpy": values([10000.0], ["J/mol"]),
        "drsm_entropy": values([0.0], ["J/(mol*K)"]), "drsm_heat_capacity_p": values([5.0], ["J/(mol*K)"]),
        "drsm_volume": values([0.5], ["J/bar"]),
        "reactants": [{"coefficient": -1, "symbol": "ZA"}, {"coefficient": 1, "symbol": "ZP"}],
        "TPMethods": methods,
        "limitsTP": {"range": False, "lowerP": 0, "lowerT": 273.15, "upperP": 1e9, "upperT": 1273.15},
        "_comment": "Reference values of the reaction ZP in tests/gems_export/gems_export_data.json; dCp and dV added",
    }


LOGK_COEFFS = {"logk_ft_coeffs": {"values": [-0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]}}


def logk_entry():
    return {"method": {"0": "logk_fpt_function"}, **LOGK_COEFFS}


FIXTURES = {
    # substances
    "s01-cp_ft_equation_saxena86": [solid("MgCO3-saxena", [method(1, "cp_ft_equation_saxena86")])],
    "s06-landau_berman88": [solid("MgCO3-berman-landau", [method(6, "landau_berman88")])],
    "s10-fluid_churakov_gottschalk": [gas("CO-cgf", [{
        "eos_churakov_gottschalk_coeffs": {"values": [3.622999906539917, 101.19999694824219, 0.10999999940395355,
                                                      0, 0, 0, 0, 0, 0, 0, 0, 0]},
        "method": {"10": "fluid_churakov_gottschalk"}}])],
    "s11-fluid_soave_redlich_kwong": [gas("CO-srk", [{**method(11, "fluid_soave_redlich_kwong"), **CRIT}])],
    "s12-fluid_sterner_pitzer": [gas("CO-stp", [{**method(12, "fluid_sterner_pitzer"), **CRIT}])],
    "s13-fluid_peng_robinson78": [gas("CO-pr78", [{**method(13, "fluid_peng_robinson78"), **CRIT}])],
    "s27-water_diel_sverj14": [water(), solute()],
    "s28-water_diel_fern97": [water(), solute()],
    "s30-water_eos_iapws95_gems": [water([method(30, "water_eos_iapws95_gems")]), solute()],
    "s33-water_pvt_zhang_duan05": [water(), solute()],
    "s35-mv_equation_dorogokupets88": [solid("MgCO3-dorogokupets", [{**method(35, "mv_equation_dorogokupets88"), **VOLUME_FPT}])],
    "s36-mv_equation_berman88": [solid("MgCO3-berman-vol", [{**method(36, "mv_equation_berman88"), **VOLUME_FPT}])],
    "s37-mv_eos_birch_murnaghan_gott97": [solid("MgCO3-bm", [{**method(37, "mv_eos_birch_murnaghan_gott97"), **BM}])],
}
# the solvent models of ids 27, 28 and 33 replace the default pair
FIXTURES["s27-water_diel_sverj14"][0]["TPMethods"] = [method(31, "water_eos_hgk84_reaktoro"), method(27, "water_diel_sverj14")]
FIXTURES["s28-water_diel_fern97"][0]["TPMethods"] = [method(31, "water_eos_hgk84_reaktoro"), method(28, "water_diel_fern97")]
FIXTURES["s33-water_pvt_zhang_duan05"][0]["TPMethods"] = [method(33, "water_pvt_zhang_duan05"), method(25, "water_diel_jnort91_reaktoro")]

REACTIONS = {
    "r05-logk_1_term_extrap1": (False, [logk_entry(), method(5, "logk_1_term_extrap1")]),
    "r06-logk_2_term_extrap": (False, [logk_entry(), method(6, "logk_2_term_extrap")]),
    "r08-logk_lagrange_interp": (False, [logk_entry(), method(8, "logk_lagrange_interp")]),
    "r09-logk_marshall_frank78": (True, [logk_entry(), {
        **method(9, "logk_marshall_frank78"),
        "dr_marshall_franck_coeffs": {"values": [-4.098, -3245.2, 2.2362e5, -3.984e7, 13.957, -1262.3, 8.5641e5],
                                      "_comment": "Ion product of water, Marshall and Franck (1981)"},
        }]),
    "r11-dr_heat_capacity_ft": (False, [{
        **method(11, "dr_heat_capacity_ft"),
        "dr_heat_capacity_ft_coeffs": {"values": [5.0, 0.01, 1000.0, 0, 0]}}, method(7, "logk_3_term_extrap")]),
    "r12-dr_volume_fpt": (False, [logk_entry(), {
        **method(12, "dr_volume_fpt"),
        "dr_volume_fpt_coeffs": {"values": [-8.900000239009387e-07, 2.2119998974862165e-12, 0, 1.8435999663779512e-05,
                                            4.1596798894261156e-08]}}]),
    "r14-logk_dolejs_manning10": (True, [logk_entry(), {
        **method(14, "logk_dolejs_manning10"),
        "dr_dolejs_manning10_coeffs": {"values": [23.6, -52.92, 10.93, -0.0463, -18.52],
                                       "_comment": "Quartz, Dolejs and Manning (2010) Table 1 (a kJ/mol)"}}]),
}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    note = "Synthetic legacy fixture of the golden suite (pytests/golden/make_synthetic_fixtures.py)"
    for name, substances in FIXTURES.items():
        data = {"thermodataset": name, "datasources": [note], "date": "2026-10-28", "substances": substances}
        (OUT / (name + ".json")).write_text(json.dumps(data, indent=1) + "\n")
    for name, (needs_solvent, methods) in REACTIONS.items():
        substances = za_zp() + ([water()] if needs_solvent else [])
        data = {"thermodataset": name, "datasources": [note], "date": "2026-10-28",
                "substances": substances, "reactions": [reaction(name.split("-", 1)[1], methods)]}
        (OUT / (name + ".json")).write_text(json.dumps(data, indent=1) + "\n")
    print("wrote", len(FIXTURES) + len(REACTIONS), "fixtures to", OUT)


if __name__ == "__main__":
    main()
