import thermofun as thermofun
import pytest as pytest
import unittest

tps_psat_al_ion_gems = [
#T(C)	P(bar)	        G0	        H0	        S0	        Cp0	        V0	    RHOwater(g/cm3)	EPSwater
(25,	1.00,	-483708.00,	-530673.00,	-325.09679,	-128.69636,	-4.5242872),#	0.99706136,	78.245146),
(0,	0.0061473168,	-491664.02,	-526451.54,	-310.25821,	-237.01532,	-4.2781218),#	0.99977821,	87.484253),
(25,	0.031720492,	-483703.62,	-530671.12,	-325.10519,	-128.68424,	-4.5257555),#	0.9970177,	78.241212),
(50,	0.1234747,	-475455.35,	-533556.06,	-334.40608,	-108.71681,	-4.7395101),#	0.98799119,	69.803017),
(75,	0.38565905,	-466993.45,	-536314.3,	-342.62086,	-114.66957,	-4.9763282),#	0.97483689,	62.239044),
(100,	1.01325,	-458325.91,	-539400.63,	-351.16531,	-134.08386,	-5.2703381),#	0.95839258,	55.49239),
(125,	2.3201744,	-449436.91,	-543102.49,	-360.7389,	-163.57125,	-5.6542994),#	0.93907264,	49.477423),
(150,	4.7571994,	-440298.84,	-547621.65,	-371.70133,	-198.81533,	-6.1616442),#	0.91705774,	44.102065),
(175,	8.9180793,	-430882.24,	-553011.69,	-384.00549,	-235.68073,	-6.8187131),#	0.89234267,	39.275148),
(200,	15.536529,	-421159.32,	-559544.66,	-398.0724,	-298.17109,	-7.6573346),#	0.86474336,	34.909378),
(225,	25.478633,	-411080.5,	-568092.51,	-415.48654,	-405.21457,	-8.7982596),#	0.83387326,	30.921883),
(250,	39.736523,	-400564.43,	-579516.68,	-437.57021,	-538.17678,	-10.47913),#	0.79907193,	27.232604),
(275,	59.431281,	-389543.19,	-593080.43,	-462.46438,	-574.16889,	-12.816218),#	0.75923621,	23.759658),
(300,	85.837873,	-378103.89,	-602659.67,	-478.96255,	-80.061385,	-14.662163),#	0.71240752,	20.408324),
(325,	120.4576,	-366754.71,	-586324.63,	-450.57684,	3059.3625,	-7.8484809),#	0.65457716,	17.040386),
(350,	165.21132,	-357073.18,	-384268.06,	-123.81849,	46654.44,	81.59456),#	0.57468752,	13.348816),
]

tps_3kbar_al_ion_gems = [
#(50,	3000,	-485696.14,	-535923.92,	-310.04298,	-150.41031,	-2.5993757),#	1.0887938,	78.08728),
(100,	3000,	-469741.66,	-541386.4,	-325.89401,	-82.809977,	-2.9357308),#	1.0624799,	63.134845),
(150,	3000,	-453186.9,	-545294.94,	-335.74537,	-74.152164,	-3.2595792),#	1.0329264,	51.632026),
(200,	3000,	-436182.05,	-549174.12,	-344.40384,	-82.218142,	-3.65646),#	1.0009181,	42.73784),
(250,	3000,	-418738.94,	-553680.69,	-353.44425,	-98.250734,	-4.1860699),#	0.96682695,	35.772923),
(300,	3000,	-400830.46,	-558843.88,	-362.86494,	-105.57338,	-4.8663121),#	0.93082544,	30.225781),
(350,	3000,	-382466.23,	-564001.03,	-371.49548,	-99.537559,	-5.6490291),#	0.8930843,	25.727622),
(400,	3000,	-363700.83,	-568848.53,	-378.97989,	-96.346953,	-6.470288),#	0.85387714,	22.020009),
(450,	3000,	-344572.27,	-573910.64,	-386.22828,	-109.15934,	-7.2920659),#	0.81362409,	18.925536),
(500,	3000,	-325061.99,	-580047.62,	-394.42313,	-138.86149,	-8.0955004),#	0.77288536,	16.323425),
(550,	3000,	-305099.96,	-588026.00,	-404.40826,	-182.37536,	-8.857843),#	0.73231472,	14.130069),
(600,	3000,	-284582.31,	-598475.52,	-416.71628,	-237.21371,	-9.546914),#	0.69258697,	12.284381),
(650,	3000,	-263385.17,	-611852.56,	-431.59833,	-298.14973,	-10.136432),#	0.65432169,	10.737849),
(700,	3000,	-241382.72,	-628212.55,	-448.8439,	-354.27276,	-10.627832),#	0.61802337,	9.4487388),
(750,	3000,	-218474.99,	-646954.95,	-467.61724,	-390.94778,	-11.064181),#	0.58404743,	8.3794023),
(800,	3000,	-194618.79,	-666756.94,	-486.51238,	-394.80296,	-11.528691),#	0.55259242,	7.4954843),
]

tps_3kbar_al_ion_thermofun = [
(100.0, 3000.0, -469737.83143536747, -541427.887498653, -326.0156146677892, -84.71017054706954, -2.9385785485472713), 
(150.0, 3000.0, -453182.3244638446, -545312.9660226026, -335.7989178985186, -74.8079561078005, -3.260194178397674), 
(200.0, 3000.0, -436175.75739772356, -549185.4363374547, -344.44119093254005, -82.21830686644749, -3.656460102899301), 
(250.0, 3000.0, -418730.77296260186, -553692.0048716286, -353.48160653546154, -98.25073287757922, -4.18606993954901), 
(300.0, 3000.0, -400820.425057345, -558855.2027997422, -362.90229135897596, -105.57338294105006, -4.866312147758318), 
(350.0, 3000.0, -382454.3310618701, -564012.3475638301, -371.5328364791145, -99.53755833386882, -5.649029140664761), 
(400.0, 3000.0, -363687.061758915, -568859.8481091476, -379.0172426654277, -96.34695228928842, -6.470288001031417), 
(450.0, 3000.0, -344556.6401469649, -573921.9610491273, -386.26563154554714, -109.15934187381937, -7.29206596952), 
(500.0, 3000.0, -325044.48534249625, -580058.9375075456, -394.46048336681, -138.8614852717252, -8.095500429352425), 
(550.0, 3000.0, -305080.588442848, -588037.3239541976, -404.445612660329, -182.3753603471309, -8.857843030600028), 
(600.0, 3000.0, -284561.07173765637, -598486.8365176627, -416.7536337742728, -237.21371234972779, -9.54691399735575), 
(650.0, 3000.0, -263362.06595737237, -611863.8813496989, -431.635688612172, -298.14972511170765, -10.136431950797526), 
(700.0, 3000.0, -241357.7465001661, -628223.8721990443, -448.88125802690064, -354.27276376084683, -10.627832497196039), 
(750.0, 3000.0, -218448.145069981, -646966.2701516029, -467.65459183074023, -390.94777821809146, -11.064180659914806), 
(800.0, 3000.0, -194590.085097332, -666768.2593295361, -486.5497318941472, -394.80296392632835, -11.528690545131157)]


tps_psat_al_ion_thermofun = [
#T(C)	P(bar)	        G0	                H0	                S0	                Cp0	                V0
(25.0, 1.0, -483708.24280802516, -530684.3184411254, -325.13414785544245, -128.69635143198644, -4.524287301503061), 
(0.0, 0.0061473168, -491667.7253655298, -526461.0853889303, -310.2911608032241, -237.08419632515674, -4.277987725604141), 
(25.0, 0.031720492, -483703.86132719327, -530682.4417346035, -325.14254890964304, -128.68423196428984, -4.5257555187105725), 
(50.0, 0.1234747, -475454.65853828867, -533567.3825576035, -334.44343051002596, -108.71680768871737, -4.739510188345039), 
(75.0, 0.38565905, -466991.82737981674, -536325.6158085571, -342.65821909734393, -114.66956364552824, -4.976328284418958), 
(100.0, 1.01325, -458323.34528331645, -539411.9483450565, -351.20266276762754, -134.08385315652342, -5.270338110941577),
(125.0, 2.3201744, -449433.417149612, -543113.8129550122, -360.77625607283704, -163.57124396977684, -5.654299423095047), 
(150.0, 4.7571994, -440294.41480356833, -547632.966445222, -371.7386794083747, -198.81532769388312, -6.1616442746626925), 
(175.0, 8.9180793, -430876.8760241684, -553023.0074487319, -384.0428472042028, -235.680734802627, -6.818713136637519), 
(200.0, 15.536529, -421153.02246792254, -559555.9816677198, -398.1097532490698, -298.1711177234206, -7.657334465482376), 
(225.0, 25.478633, -411073.26942823856, -568103.8216775215, -415.5238839692519, -405.21473312646486, -8.798258565759907), 
(250.0, 39.736523, -400556.2742924387, -579527.9973412296, -437.60755729483077, -538.1778681110522, -10.47912866389862), 
(275.0, 59.431281, -389534.1064722969, -593091.8526774439, -462.50190049283356, -574.1775471814568, -12.816229642891443), 
(300.0, 85.837873, -378093.86936207017, -602671.2019895697, -479.00201199947537, -80.15322660861946, -14.662389102919917), 
(325.0, 120.4576, -366743.572879816, -586327.219322251, -450.63331437337644, 3059.3076791226126, -7.85122170632791), 
(350.0, 165.21132, -357061.32648562285, -384274.6953558837, -123.84825390397269, 46654.50019076753, 81.59597584418344),
]

tps_psat_sio2_gems = [
(0,	0.0061473168,	-831031.54,	-863724.84,	124.64846,	-933.87348,	1.925035),#	0.99977821	87.484253
(25,	0.031720492,	-833412.56,	-877702.96,	75.303937,	-318.41846,	1.6061839),#	0.9970177	78.241212
(50,	0.1234747,	-835052.61,	-882809.8,	58.750065,	-120.28684,	1.4502793),#	0.98799119	69.803017
(75,	0.38565905,	-836439.66,	-884596.64,	53.383001,	-33.012576,	1.353989),#	0.97483689	62.239044
(100,	1.01325,	-837759.6,	-884796.89,	52.807131,	12.408843,	1.2842922),#	0.95839258	55.49239
(125,	2.3201744,	-839096.3,	-884131.42,	54.520022,	38.308832,	1.2264575),#	0.93907264	49.477423
(150,	4.7571994,	-840490.29,	-882961.61,	57.357784,	53.487794,	1.1716236),#	0.91705774	44.102065
(175,	8.9180793,	-841960.67,	-881500.41,	60.6996,	61.677479,	1.1122506),#	0.89234267	39.275148
(200,	15.536529,	-843514.37,	-879899.78,	64.159054,	64.12188,	1.0391902),#	0.86474336	34.909378
(225,	25.478633,	-845150.13,	-878300.99,	67.432319,	60.113988,	0.93783509),#	0.83387326	30.921883
(250,	39.736523,	-846859.57,	-876885.71,	70.182809,	45.960438,	0.77993303),#	0.79907193	27.232604
(275,	59.431281,	-848625.51,	-875973.78,	71.867187,	10.249988,	0.50105936),#	0.75923621	23.759658
(300,	85.837873,	-850415.37,	-876312.85,	71.263704,	-87.054334,	-0.07746856),#	0.71240752	20.408324
(325,	120.4576,	-852159.36,	-880328.56,	64.487286,	-449.44611,	-1.6250326),#	0.65457716	17.040386
(350,	165.21132,	-853649.42,	-902227.42,	29.149126,	-3510.8132,	-8.7584118),#	0.57468752	13.348816
]

tps_psat_sio2_thermofun = [
(0.0, 0.0061473168, -831030.3188146922, -863716.0524024032, 124.6807219926376, -934.3384864799659, 1.9252323606816022), 
(25.0, 0.031720492, -833412.5666883292, -877703.4889468716, 75.30219869682232, -318.4184633089192, 1.6061839191129914), 
(50.0, 0.1234747, -835052.5822357356, -882810.3333148039, 58.74832653854792, -120.28684204835005, 1.4502793003907564), 
(75.0, 0.38565905, -836439.5796927373, -884597.166136166, 53.3812619749284, -33.0125788431724, 1.3539890127799956), 
(100.0, 1.01325, -837759.4804451603, -884797.4212317914, 52.80539196936579, 12.408840865197613, 1.2842921789304471), 
(125.0, 2.3201744, -839096.1334719093, -884131.9521382651, 54.51828239016465, 38.30882976952072, 1.2264575358315264), 
(150.0, 4.7571994, -840490.079056353, -882962.1415154422, 57.35604476169336, 53.48779094237014, 1.1716236071845307), 
(175.0, 8.9180793, -841960.4153832584, -881500.9419082759, 60.69786070508163, 61.67747611309854, 1.1122505995168628), 
(200.0, 15.536529, -843514.073409936, -879900.3130293209, 64.15731413001181, 64.1218792055988, 1.039190259960612), 
(225.0, 25.478633, -845149.7962453063, -878301.5250000455, 67.43058124111359, 60.114005937850585, 0.9378352210955291), 
(250.0, 39.736523, -846859.1926521804, -876886.2344663447, 70.18107805760451, 45.96056456930593, 0.7799337945280096), 
(275.0, 59.431281, -848625.0841766259, -875974.2887441505, 71.86548979745564, 10.250827913294483, 0.5010637020425329), 
(300.0, 85.837873, -850414.9152243602, -876313.2503947007, 71.26221343393401, -87.04718958113303, -0.07743868033917634), 
(325.0, 120.4576, -852158.8792203282, -880328.2428872865, 64.48701685704506, -449.37251923954403, -1.6248148474838855), 
(350.0, 165.21132, -853648.8666352547, -902228.2166598319, 29.146951416870177, -3510.8703151995196, -8.758496107121664),
]

tps_3kbar_sio2_gems = [
(50,	3000,	-830476.85,	-872194.18,	77.440657,	-241.60186,	1.5708946),#	1.0887938	78.08728
(100,	3000,	-833815.80,	-877597.65,	61.531343,	-24.024199,	1.3329949),#	1.0624799	63.134845
(150,	3000,	-836902.11,	-876879.81,	63.250797,	42.217042,	1.209557),#	1.0329264	51.632026
(200,	3000,	-840215.86,	-873981.65,	69.695623,	70.211287,	1.1268183),#	1.0009181	42.73784
(250,	3000,	-843893.44,	-870088.13,	77.506605,	84.010765,	1.0593957),#	0.96682695	35.772923
(300,	3000,	-847969.74,	-865689.98,	85.530898,	91.118373,	0.9947072),#	0.93082544	30.225781
(350,	3000,	-852442.33,	-861037.68,	93.311285,	94.499323,	0.92428188),#	0.8930843	25.727622
(400,	3000,	-857293.59,	-856277.48,	100.65867,	95.619568,	0.84099337),#	0.85387714	22.020009
(450,	3000,	-862499.87,	-851497.38,	107.50849,	95.436455,	0.73827101),#	0.81362409	18.925536
(500,	3000,	-868036.23,	-846742.09,	113.86718,	94.756488,	0.61032335),#	0.77288536	16.323425
(550,	3000,	-873879.28,	-842017.42,	119.78878,	94.332262,	0.45302813),#	0.73231472	14.130069
(600,	3000,	-880009.25,	-837293.43,	125.36002,	94.828038,	0.26516213),#	0.69258697	12.284381
(650,	3000,	-886411.19,	-832511.14,	130.6855,	96.722491,	0.049508279),#	0.65432169	10.737849
(700,	3000,	-893075.49,	-827594.55,	135.87136,	100.20286,	-0.1867176),#	0.61802337	9.4487388
(750,	3000,	-899997.53,	-822466.9,	141.00856,	105.11002,	-0.43256166),#	0.58404743	8.3794023
(800,	3000,	-907176.60,	-817067.32,	146.15995,	110.98553,	-0.67514825),#	0.55259242	7.4954843
]

tps_3kbar_sio2_thermofun = [
(50.0, 3000.0, -830476.8208154694, -872194.7073466043, 77.43891774366374, -241.6018622445966, 1.570894612928876), 
(100.0, 3000.0, -833815.6803065118, -877598.1816584462, 61.52960323747993, -24.024201038150707, 1.332994916359956), 
(150.0, 3000.0, -836901.9024131005, -876880.3365756492, 63.24905739678856, 42.2170404064927, 1.2095569928621477), 
(200.0, 3000.0, -840215.5666781921, -873982.1788397763, 69.69388278223782, 70.21128524641327, 1.1268182676060154), 
(250.0, 3000.0, -843893.0624949102, -870088.6652880274, 77.50486477469677, 84.01076276976225, 1.059395719705037), 
(300.0, 3000.0, -847969.2729954666, -865690.5091923374, 85.52915746860143, 91.11837100777306, 0.9947072036574864), 
(350.0, 3000.0, -852441.7789592376, -861038.209096781, 93.30954451168499, 94.4993206723989, 0.9242818765480921), 
(400.0, 3000.0, -857292.9467291291, -856278.00703365, 100.65693009801521, 95.61956565971903, 0.8409933635361938), 
(450.0, 3000.0, -862499.1415070419, -851497.9068126504, 107.50675170350762, 95.43645204985076, 0.7382709986806838), 
(500.0, 3000.0, -868035.4102583308, -846742.6209939258, 113.86543628585007, 94.75648518204659, 0.6103233302817117), 
(550.0, 3000.0, -873878.3823258805, -842017.9508867031, 119.78704274941052, 94.33225936509407, 0.45302809557709517), 
(600.0, 3000.0, -880008.2648712863, -837293.959274512, 125.35827566486199, 94.82803525713524, 0.2651620827106574), 
(650.0, 3000.0, -886410.1132706174, -832511.6707925296, 130.68376242007017, 96.72248749171088, 0.04950821571345728), 
(700.0, 3000.0, -893074.3225209342, -827595.0787217184, 135.86961578298892, 100.20286162365888, -0.1867176844509069), 
(750.0, 3000.0, -899996.2800760249, -822467.431335693, 141.00681380084242, 105.11001569804729, -0.43256176190337065), 
(800.0, 3000.0, -907175.2598261723, -817067.8506783115, 146.15820896227066, 110.98552344536564, -0.6751483711924663)]

class TestHKFGems(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/Substances/Solute/database-thermofun.json')
    
    def almost_equal(self, table, substance, dif):
        results = []
        for row in table:
            T, P, G, H, S, Cp, V = row 
            T = T+273.15
            P = P*1e5

            tps = self.engine.thermoPropertiesSubstance(T,P, substance)
            self.assertAlmostEqual(tps.gibbs_energy.val, G, delta=dif['G'])
            self.assertAlmostEqual(tps.enthalpy.val, H, delta=dif['H'])
            self.assertAlmostEqual(tps.entropy.val, S, delta=dif['S'])
            self.assertAlmostEqual(tps.heat_capacity_cp.val, Cp, delta=dif['Cp'])
            self.assertAlmostEqual(tps.volume.val, V, delta=dif['V'])

        #     row1 = T-273.15, P/1e5, tps.gibbs_energy.val, tps.enthalpy.val, tps.entropy.val, tps.heat_capacity_cp.val, tps.volume.val
        #     results.append(row1)

        # print(results)
        # assert False


    def test_psat_al_ion(self):
        self.almost_equal(tps_psat_al_ion_gems, "Al+3", {'G': 12, 'H': 12, 'S': 0.2, 'Cp': 0.1, 'V': 0.1})
        self.almost_equal(tps_psat_al_ion_thermofun, "Al+3", {'G': 0., 'H': 0., 'S': 0., 'Cp': 0., 'V': 0.})

    def test_3kbar_al_ion(self):
        self.almost_equal(tps_3kbar_al_ion_gems, "Al+3", {'G': 30, 'H': 45, 'S': 0.2, 'Cp': 2, 'V': 0.1})
        self.almost_equal(tps_3kbar_al_ion_thermofun, "Al+3", {'G': 0., 'H': 0., 'S': 0., 'Cp': 0., 'V': 0.})

    def test_psat_sio2(self):
        self.almost_equal(tps_psat_sio2_gems, "SiO2@", {'G': 2., 'H': 10., 'S': 0.05, 'Cp': 0.5, 'V': 0.01})
        self.almost_equal(tps_psat_sio2_thermofun, "SiO2@", {'G': 0., 'H': 0., 'S': 0., 'Cp': 0., 'V': 0.})

    def test_3kbar_sio2(self):
        self.almost_equal(tps_3kbar_sio2_gems, "SiO2@", {'G': 2., 'H': 1., 'S': 0.002, 'Cp': 0.001, 'V': 0.001})
        self.almost_equal(tps_3kbar_sio2_thermofun, "SiO2@", {'G': 0., 'H': 0., 'S': 0., 'Cp': 0., 'V': 0.})
        



