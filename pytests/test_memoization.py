# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

import thermofun as thermofun
import unittest

# Regression coverage for the memoization cache in ThermoFun/OptimizationUtils.h,
# exercised through the public Python API (which is the same path GEMS3K uses via
# TNode::load_all_thermodynamic_from_thermo - see CLAUDE.md).
#
# ThermoEngine's memoized property functions take pressure as an in-out C++
# reference: P == 0 means "at the saturation pressure", and the solvent model
# writes the computed Psat back through it. pybind11 cannot reflect that
# write-back to a Python float, so these tests cannot observe P directly (that
# half is covered by tests/memoization/src/main.cpp instead). What they can and
# do check is the symptom that made the underlying bug into a real-world
# incident: with the cache keyed on a dangling reference, a substance's
# properties could come back as another substance's - most dramatically, every
# entry from one pass collapsing onto the last one computed. So every symbol
# queried here must keep returning its own value, repeatably, regardless of what
# else was queried in between.


class TestMemoization(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/test-thermoengine-thermofun.json')
        self.engine2 = thermofun.ThermoEngine('pytests/test-aq17-gem-lma-thermofun.json')

    def test_saturation_pressure_substance_properties_stay_distinct_and_stable(self):
        T = 473.15
        symbols = ['Ca+2', 'Mg+2', 'Na+', 'K+', 'Cl-', 'HCO3-', 'CO3-2', 'H4SiO4@']

        # Independent ground truth: its own engine, its own cache, queried once
        # per symbol in a fixed order - decoupled from whichever access pattern
        # exercises self.engine2 below. Not a numeric literal pinned from some
        # other machine's build: the underlying computation is a deterministic
        # function of (T, P, symbol), so it must reproduce bit-for-bit on every
        # build/environment, which a hardcoded constant is not guaranteed to do
        # (dependency versions differ between a local conda env and CI's
        # conda-devenv one - this caught a real CI failure).
        baseline_engine = thermofun.ThermoEngine('pytests/test-aq17-gem-lma-thermofun.json')
        baseline = {sym: baseline_engine.thermoPropertiesSubstance(T, 0, sym).gibbs_energy.val
                    for sym in symbols}

        # Pass 1, in order; pass 2, reversed - a different sequence of preceding
        # calls is exactly what let a dangling-reference cache key return a
        # neighbour's value.
        pass1 = {sym: self.engine2.thermoPropertiesSubstance(T, 0, sym).gibbs_energy.val
                 for sym in symbols}
        pass2 = {sym: self.engine2.thermoPropertiesSubstance(T, 0, sym).gibbs_energy.val
                 for sym in reversed(symbols)}

        for sym in symbols:
            assert pass1[sym] == baseline[sym]
            assert pass2[sym] == baseline[sym]

        # No two distinct substances collapsed onto the same cached entry.
        assert len(set(baseline.values())) == len(symbols)

    def test_saturation_pressure_reaction_properties_stay_distinct_and_stable(self):
        T = 298.15
        symbols = ['Meionite-Ca', 'Gedrite-Mg', 'Tschermakite-Mg', 'Pargasite-Mg',
                   'Pyrope', 'Grossular', 'Forsterite']

        # Same rationale as the substance test above, for
        # ThermoPropertiesReactionFunction, the fourth memoized function.
        baseline_engine = thermofun.ThermoEngine('pytests/test-aq17-gem-lma-thermofun.json')
        baseline = {sym: baseline_engine.thermoPropertiesReaction(T, 0, sym).log_equilibrium_constant.val
                    for sym in symbols}

        pass1 = {sym: self.engine2.thermoPropertiesReaction(T, 0, sym).log_equilibrium_constant.val
                 for sym in symbols}
        pass2 = {sym: self.engine2.thermoPropertiesReaction(T, 0, sym).log_equilibrium_constant.val
                 for sym in reversed(symbols)}

        for sym in symbols:
            assert pass1[sym] == baseline[sym]
            assert pass2[sym] == baseline[sym]

        assert len(set(baseline.values())) == len(symbols)

    def test_repeated_call_same_symbol_matches_first_call(self):
        # A cache hit must return exactly what the (memoized) miss returned -
        # this is memoize()/memoizeN() acting as a pure cache, independent of
        # the P write-back tested on the C++ side.
        first = self.engine.thermoPropertiesSubstance(873.15, 5000e5, "Quartz").gibbs_energy.val
        for _ in range(5):
            again = self.engine.thermoPropertiesSubstance(873.15, 5000e5, "Quartz").gibbs_energy.val
            assert again == first
