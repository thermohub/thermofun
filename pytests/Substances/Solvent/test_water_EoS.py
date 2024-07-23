import thermofun as thermofun
import pytest as pytest
import unittest


class TestWaterEoS(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/Substances/Solvent/water-thermofun.json')
    
    def test_water_electro_properties(self):
        P = 300e5 # region crit
        T = 420+273.15
        assert self.engine.propertiesSolvent(T,P, "H2O@", 1).density.val == pytest.approx(203.47268444814958, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 1).epsilon.val == pytest.approx(3.1942838457177976, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@", 0).density.val == pytest.approx(203.47268444814958, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 0).epsilon.val == pytest.approx(3.1942838457177976, 1e-5, 1e-14)

        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 1).density.val == pytest.approx(203.2389910219359, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 1).epsilon.val == pytest.approx(3.190715033274149, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 0).density.val == pytest.approx(203.2389910219359, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 0).epsilon.val == pytest.approx(3.190715033274149, 1e-5, 1e-14)

        P = 40e5 # vapour
        T = 350+273.15
        assert self.engine.propertiesSolvent(T,P, "H2O@", 1).density.val == pytest.approx(15.05176552064709, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 1).epsilon.val == pytest.approx(1.1120564097130872, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@", 0).density.val == pytest.approx(452.42695689302792, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 0).epsilon.val == pytest.approx(9.4564825264140211, 1e-5, 1e-14)

        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 1).density.val == pytest.approx(15.043737111107781, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 1).epsilon.val == pytest.approx(1.1119933554710066, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 0).density.val == pytest.approx(15.043737111107781, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 0).epsilon.val == pytest.approx(1.1119933554710066, 1e-5, 1e-14)

        # region 3
        P = 300e5
        T = 380+273.15
        assert self.engine.propertiesSolvent(T,P, "H2O@", 1).density.val == pytest.approx(533.9645612493881, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 1).epsilon.val == pytest.approx(11.315069043656928, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@", 0).density.val == pytest.approx(533.9645612493881, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 0).epsilon.val == pytest.approx(11.315069043656928, 1e-5, 1e-14)

        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 1).density.val == pytest.approx(533.9301819281667, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 1).epsilon.val == pytest.approx(11.31400838132288, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 0).density.val == pytest.approx(533.9301819281667, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 0).epsilon.val == pytest.approx(11.31400838132288, 1e-5, 1e-14)

        # region 2
        P = 600e5
        T = 450+273.15
        assert self.engine.propertiesSolvent(T,P, "H2O@", 1).density.val == pytest.approx(479.86864831430904, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 1).epsilon.val == pytest.approx(8.635464564374157, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@", 0).density.val == pytest.approx(479.86864831430904, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@", 0).epsilon.val == pytest.approx(8.635464564374157, 1e-5, 1e-14)

        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 1).density.val == pytest.approx(479.5064053309655, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 1).epsilon.val == pytest.approx(8.626211060320887, 1e-5, 1e-14)
        assert self.engine.propertiesSolvent(T,P, "H2O@reak", 0).density.val == pytest.approx(479.5064053309655, 1e-5, 1e-14)
        assert self.engine.electroPropertiesSolvent(T,P, "H2O@reak", 0).epsilon.val == pytest.approx(8.626211060320887, 1e-5, 1e-14)