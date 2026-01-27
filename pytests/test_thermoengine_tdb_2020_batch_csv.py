import csv
import math
import pytest
from pathlib import Path
import thermofun as thermofun


def read_csv(path):
    with open(path, newline="") as f:
        reader = csv.reader(f)
        rows = list(reader)
    return rows


def assert_csv_equal(generated, reference, atol=1e-10, rtol=1e-10):
    g = read_csv(generated)
    r = read_csv(reference)

    # Compare shape
    assert len(g) == len(r), f"Row count mismatch: {len(g)} vs {len(r)}"
    for i, (g_row, r_row) in enumerate(zip(g, r)):
        assert len(g_row) == len(r_row), f"Column count mismatch in row {i}"

        for j, (g_val, r_val) in enumerate(zip(g_row, r_row)):
            # Try numeric comparison
            try:
                g_num = float(g_val)
                r_num = float(r_val)
                if math.isnan(g_num) and math.isnan(r_num):
                    continue
                assert math.isclose(g_num, r_num, abs_tol=atol, rel_tol=rtol), \
                    f"Numeric mismatch at row {i}, col {j}: {g_num} vs {r_num}"
            except ValueError:
                # Compare as strings
                assert g_val == r_val, \
                    f"String mismatch at row {i}, col {j}: '{g_val}' vs '{r_val}'"


@pytest.fixture
def engine():
    db_path = Path(__file__).parent / "PsiTDB2020-subset-thermofun.json"
    return thermofun.ThermoEngine(str(db_path))

def test_batch_csv_outputs(engine, tmp_path):

    batch = thermofun.ThermoBatch(engine)

    batch.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
    batch.setSolventSymbol("H2O(l)")

    prefs = thermofun.BatchPreferences()
    prefs.writeNaNifNotDefinedValue = True
    batch.setBatchPreferences(prefs)

    # Output paths
    st2 = tmp_path / "results_st.csv"
    sp2 = tmp_path / "results_sp.csv"
    rt2 = tmp_path / "results_rt.csv"
    rp2 = tmp_path / "results_rp.csv"

    # === Generate CSVs ===

    batch.thermoPropertiesSubstance(
        [[25, 1], [40, 1]],
        ["Ag+", "Ag(aq)", "H3PO4(aq)", "(UO2)3(PO4)2w4(cr)", "(HgOH)3PO4(s)",
         "Ca+2", "CaCO3(aragonite)", "FeOOH(gamma)", "CO2(g)", "Fe4(OH)8Clwn(s)"],
        ["gibbs_energy", "entropy", "volume", "enthalpy", "heat_capacity_cp"]
    ).toCSV(str(st2))

    batch.thermoPropertiesSubstance(
        [[25, 1], [25, 5000]],
        ["Ag+", "Ag(aq)", "H3PO4(aq)", "(UO2)3(PO4)2w4(cr)", "(HgOH)3PO4(s)",
         "Ca+2", "CaCO3(aragonite)", "FeOOH(gamma)", "CO2(g)", "Fe4(OH)8Clwn(s)"],
        ["gibbs_energy", "entropy", "volume", "enthalpy", "heat_capacity_cp"]
    ).toCSV(str(sp2))

    batch.thermoPropertiesReaction(
        [[25, 1], [40, 1]],
        ["Ag(aq)", "H3PO4(aq)", "(UO2)3(PO4)2w4(cr)", "(HgOH)3PO4(s)",
         "CaCO3(aragonite)", "Fe4(OH)8Clwn(s)"],
        ["logKr", "reaction_gibbs_energy", "reaction_entropy",
         "reaction_volume", "reaction_enthalpy", "reaction_heat_capacity_cp"]
    ).toCSV(str(rt2))

    batch.thermoPropertiesReaction(
        [[25, 1], [25, 5000]],
        ["Ag(aq)", "H3PO4(aq)", "(UO2)3(PO4)2w4(cr)", "(HgOH)3PO4(s)",
         "CaCO3(aragonite)", "Fe4(OH)8Clwn(s)"],
        ["logKr", "reaction_gibbs_energy", "reaction_entropy",
         "reaction_volume", "reaction_enthalpy", "reaction_heat_capacity_cp"]
    ).toCSV(str(rp2))

    # === Compare with reference CSVs ===

    ref_dir = Path(__file__).parent / "reference/tdb2020"

    assert_csv_equal(st2, ref_dir / "results_st.csv")
    assert_csv_equal(sp2, ref_dir / "results_sp.csv")
    assert_csv_equal(rt2, ref_dir / "results_rt.csv")
    assert_csv_equal(rp2, ref_dir / "results_rp.csv")
