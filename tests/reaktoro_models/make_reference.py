"""Makes the database and the reference values of the test reaktoro-models with Reaktoro (python, version 2.13).
Run:  python make_reference.py <a ThermoFun database with "elements", for example pytests/test-thermoengine-thermofun.json>  -> reaktoro_models_db.json, reaktoro_reference.csv
Entropy and heat capacity of Reaktoro are not given by these models: they are taken from the derivatives of G0 in T."""
import json, sys, csv
import reaktoro as rkt

elements = json.load(open(sys.argv[1]))['elements']

def base(sym, formula, state, cls, G, H, S, Cp, V, methods, extra=None):
    d = {"name": sym, "symbol": sym, "formula": formula, "formula_charge": 0,
         "mass_per_mole": {"values": [100.0], "units": ["g/mol"]}, "aggregate_state": state, "class_": cls,
         "Tst": 298.15, "Pst": 1e5,
         "sm_gibbs_energy": {"values": [G], "units": ["J/mol"], "errors": [0]},
         "sm_enthalpy": {"values": [H], "units": ["J/mol"], "errors": [0]},
         "sm_entropy_abs": {"values": [S], "units": ["J/(mol*K)"], "errors": [0]},
         "sm_heat_capacity_p": {"values": [Cp], "units": ["J/(mol*K)"], "errors": [0]},
         "sm_volume": {"values": [V], "units": ["J/bar"], "errors": [0]}, "TPMethods": methods}
    if extra: d.update(extra)
    return d

def props(m, T, P_bar):
    p = rkt.StandardThermoProps(); m.apply(p, T, P_bar * 1e5); return p

def derivs(m, T, P_bar, h=0.01):
    g0 = float(props(m, T, P_bar).G0); gm = float(props(m, T - h, P_bar).G0); gp = float(props(m, T + h, P_bar).G0)
    return -(gp - gm) / (2 * h), -T * (gp - 2 * g0 + gm) / h ** 2

subs, rows = [], []
# tolerances: G, H, S, Cp, V (negative: not compared)
def add(sym, m, pts, tol, S=True, Cp=True, V=True, H=True):
    for T, P in pts:
        p = props(m, T, P)
        s, c = derivs(m, T, P)
        rows.append([sym, T, P, float(p.G0), float(p.H0) if H else 0, s if S else 0, float(p.Cp0) if Cp else c, float(p.V0) * 1e5 if V else 0] + tol)

# ---- Holland and Powell (2011) Tait EoS; forsterite-like
a, b, c, d = 238.64, -0.0020013, -1.1600e6, -2054.8
pa = rkt.StandardThermoModelParamsHollandPowell()
pa.Gf, pa.Hf, pa.Sr, pa.Vr, pa.a, pa.b, pa.c, pa.d, pa.alpha0, pa.kappa0, pa.kappa0p, pa.kappa0pp, pa.numatoms = \
    -2056.0e3, -2172.4e3, 95.1, 4.366e-5, a, b, c, d, 2.85e-5, 1.285e11, 3.84, -2.99e-11, 7
mt = rkt.StandardThermoModelHollandPowell(pa)
Cp0 = a + b * 298.15 + c / 298.15 ** 2 + d / 298.15 ** 0.5
subs.append(base("Fo", "Mg2SiO4", {"3": "AS_CRYSTAL"}, {"0": "SC_COMPONENT"}, -2056.0e3, -2172.4e3, 95.1, Cp0, 4.366,
    [{"method": {"0": "cp_ft_equation"}, "m_heat_capacity_ft_coeffs": {"values": [a, b, c, d]}, "limitsTP": {"lowerT": 273.15, "upperT": 2273.15, "range": True}},
     {"method": {"39": "mv_eos_tait_hp11"}, "eos_tait_hp11_coeffs": {"values": [3.84, -2.99e-11 * 1e8, 7], "units": ["1", "1/kbar", "1"]}}],
    {"m_expansivity": {"values": [2.85e-5], "units": ["kbar"]}, "m_compressibility": {"values": [1285.0], "units": ["1e-05/K"]}}))
# G of ThermoFun is Gf at the reference T and P; Reaktoro adds the integral of V dP from 0 bar (about -V*1 bar): 5 J/mol
add("Fo", mt, [(298.15, 1), (298.15, 2000), (500, 5000), (800, 10000), (1000, 20000)], [5.0, -1, 2e-3, 0.05, 1e-5], Cp=False, H=False)

# ---- Extended UNIQUAC
pu = rkt.StandardThermoModelParamsExtendedUNIQUAC()
pu.Gr, pu.Hr, pu.Sr, pu.Vr, pu.a, pu.b, pu.c, pu.alpha, pu.beta, pu.Theta = -300.0, -350.0, 100.0, 3e-5, 80.0, 0.05, -10000.0, 3e-5, 2e-9, 200.0
mu = rkt.StandardThermoModelExtendedUNIQUAC(pu)
subs.append(base("UQ", "NaCl", {"4": "AS_AQUEOUS"}, {"2": "SC_AQSOLUTE"}, -300e3, -350e3, 100.0, 80.0 + 0.05 * 298.15 - 10000.0 / 98.15, 3.0,
    [{"method": {"44": "cp_ft_equation_uniquac"}, "eos_extended_uniquac_coeffs": {"values": [80.0, 0.05, -10000.0, 3e-5, 2e-9, 200.0]}}]))
# the gas constant of ThermoFun and of Reaktoro differ in the sixth digit
class Cp_(object): pass
for T, P in [(298.15, 1), (350, 1), (450, 1), (350, 500), (450, 2000)]:
    p = props(mu, T, P); s, _ = derivs(mu, T, P)
    rows.append(["UQ", T, P, float(p.G0), float(p.H0), s, float(p.Cp0), 0.0, 0.01, 0.01, 1e-4, 1e-3, -1])

# ---- NASA polynomials of CO2 (NASA-9 form)
A = [(200, 1000, [4.943650540E+04, -6.264116010E+02, 5.301725240E+00, 2.503813816E-03, -2.127308728E-07, -7.689988780E-10, 2.849677801E-13], [-4.528198460E+04, -7.048279440E+00]),
     (1000, 6000, [1.176962419E+05, -1.788791477E+03, 8.291523190E+00, -9.223156780E-05, 4.863676880E-09, -1.891053312E-12, 6.330036590E-16], [-3.908350590E+04, -2.652669281E+01])]
pn = rkt.StandardThermoModelParamsNasa()
polys = []
for lo, hi, co, bb in A:
    q = rkt.StandardThermoModelParamsNasaPolynomial(); q.Tmin, q.Tmax = lo, hi
    q.a1, q.a2, q.a3, q.a4, q.a5, q.a6, q.a7 = co; q.b1, q.b2 = bb; polys.append(q)
pn.polynomials = polys
mn = rkt.StandardThermoModelNasa(pn)
subs.append(base("NS", "CO2", {"0": "AS_GAS"}, {"1": "SC_GASFLUID"}, 0.0, 0.0, 0.0, 0.0, 0.0,
    [{"method": {"45": "cp_nasa_polynomials"}, "limitsTP": {"lowerT": lo, "upperT": hi, "range": True}, "nasa_polynomial_coeffs": {"values": co + bb}} for lo, hi, co, bb in A]))
# the gas constant of ThermoFun is 8.31451, that of Reaktoro 8.3144598: relative difference 6e-6
for T, P in [(298.15, 1), (500, 1), (999, 1), (1000, 1), (1500, 100), (3000, 1)]:
    p = props(mn, T, P)
    rows.append(["NS", T, P, float(p.G0), float(p.H0), 0.0, float(p.Cp0), 0.0, abs(float(p.G0)) * 8e-6 + 0.01, abs(float(p.H0)) * 8e-6 + 0.01, -1, 1e-3, -1])

# ---- interpolation in T and P
Ts, Ps = [300., 400., 500.], [1e5, 1e7, 5e7]
g = lambda T, P: -1.0e5 - 120.0 * T + 0.002 * T * T + 2.5e-5 * (P - 1e5)
hh = lambda T, P: -1.2e5 + 30.0 * T + 1.0e-5 * (P - 1e5)
vv = lambda T, P: 2.0e-5 * (1 + 1e-4 * (T - 300)) * (1 - 1e-11 * (P - 1e5))
cc = lambda T, P: 40.0 + 0.01 * T
R2 = lambda f: [[f(T, P) for T in Ts] for P in Ps]
pi = rkt.StandardThermoModelParamsInterpolation(); pi.temperatures = Ts; pi.pressures = Ps
pi.G0, pi.H0, pi.V0, pi.Cp0 = R2(g), R2(hh), R2(vv), R2(cc)
mi = rkt.StandardThermoModelInterpolation(pi)
flat = lambda f: [f(T, P) for P in Ps for T in Ts]
subs.append(base("IT", "SiO2", {"3": "AS_CRYSTAL"}, {"0": "SC_COMPONENT"}, g(300, 1e5), hh(300, 1e5), 0.0, cc(300, 1e5), vv(300, 1e5) * 1e5,
    [{"method": {"46": "standard_thermo_interpolation"}, "tp_table": {"temperatures": Ts, "pressures": Ps, "gibbs_energy": flat(g), "enthalpy": flat(hh),
      "volume": flat(vv), "heat_capacity_cp": flat(cc), "reference_pressure": 1e5}}]))
for T, P in [(300, 1), (350, 1), (450, 50), (400, 100), (500, 500), (450, 300)]:
    p = props(mi, T, P)
    rows.append(["IT", T, P, float(p.G0), float(p.H0), 0.0, float(p.Cp0), float(p.V0) * 1e5, 1e-3, 1e-3, -1, 1e-6, 1e-6])

json.dump({"elements": elements, "substances": subs, "reactions": []}, open("reaktoro_models_db.json", "w"), indent=1)
with open("reaktoro_reference.csv", "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["symbol", "T_K", "P_bar", "G", "H", "S", "Cp", "V", "tolG", "tolH", "tolS", "tolCp", "tolV"])
    for r in rows: w.writerow([r[0]] + ["%.12g" % x for x in r[1:]])
