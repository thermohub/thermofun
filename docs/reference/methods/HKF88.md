# HKF88

Revised HKF equation of state.

Calculates the properties of an aqueous species from the revised Helgeson-Kirkham-Flowers equation of state.

Source: Tanger and Helgeson (1988); Shock et al. (1992).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | aqueous_solute |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `solute_hkf88_gems`, `solute_hkf88_reaktoro` |
| Other names | `HKFReaktoro`, `HKFGems` |

## Implementations

`reaktoro`, `gems`. The default is `reaktoro`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | cal/(mol.bar) | J/(mol.Pa) | yes |  | pressure coefficient a1 |
| a2 | cal/mol | J/mol | yes |  | pressure coefficient a2 |
| a3 | cal.K/(mol.bar) | J.K/(mol.Pa) | yes |  | pressure coefficient a3 |
| a4 | cal.K/mol | J.K/mol | yes |  | pressure coefficient a4 |
| c1 | cal/(mol.K) | J/(mol.K) | yes |  | temperature coefficient c1 |
| c2 | cal.K/mol | J.K/mol | yes |  | temperature coefficient c2 |
| omega | cal/mol | J/mol | yes |  | conventional Born coefficient |

## Built-in coefficients

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Implementation `reaktoro`.

Units of the values: T: K, P: bar, rho: g/cm3, energy: cal/mol. They are not converted.

Universal constants of the HKF equation of state of aqueous species and of its g function, Reaktoro digits. eta in A cal/mol; theta in K; psi and Pr in bar; Tr in K; zeta in A; Zr, Yr: Born functions at Tr, Pr; ag, bg: g function in the units of the paper; af: g function in the shaded region. Dmax in g/cm3: above this density of water the model warns that it is out of its range. The two implementations have different digits of these numbers, both kept to reproduce each code.

Source: Tanger and Helgeson (1988); Shock et al. (1992).

Implementation `gems`.

Units of the values: T: K, P: bar, rho: g/cm3, energy: cal/mol. They are not converted.

Universal constants of the HKF equation of state of aqueous species and of its g function, GEMS digits. eta in A cal/mol; theta in K; psi and Pr in bar; Tr in K; zeta in A; Zr, Yr: Born functions at Tr, Pr; ag, bg: g function in the units of the paper; af: g function in the shaded region. Dmax in g/cm3: above this density of water the model warns that it is out of its range. The two implementations have different digits of these numbers, both kept to reproduce each code.

Source: Tanger and Helgeson (1988); Shock et al. (1992).

Names: `eta`, `theta`, `psi`, `Tr`, `Pr`, `Zr`, `Yr`, `zeta`, `gr`, `ag1` to `ag3`, `bg1` to `bg3`, `af1` to `af3`, `Dmax`.

## Named outputs

Besides G, H, S, Cp and V, the method gives these terms and intermediate functions of the model, in SI units, on request: `engine.auxiliaryOutputs(symbol, T, P)` gives them as rows (the column is `HKF88:<name>`), a table setup takes them as columns with the same name, and `engine.explain` has them for each method.

| Name | SI unit | What it is |
|---|---|---|
| `g` | Ao | the g function of Shock et al. (1992): the shift of the effective electrostatic radius with T and P (0 for a neutral species) |
| `re` | Ao | the effective electrostatic radius of the species at T and P (0 for a neutral species) |
| `omega` | J/mol | the conventional Born coefficient at T and P |
| `omega_reference` | J/mol | the Born coefficient at the reference T and P (the parameter omega) |
| `born_G` | J/mol | the Born (solvation) term of G: -w (Z + 1) + wr (Zr + 1) + wr Yr (T - Tr) |
| `born_H` | J/mol | the Born term of H |
| `born_S` | J/(mol.K) | the Born term of S |
| `born_Cp` | J/(mol.K) | the Born term of Cp |
| `born_V` | m3/mol | the Born term of V: -w Q - (Z + 1) dw/dP |

## Example

From Al(OH)2+ in `aq17-thermofun.json`.

```yaml
- method: HKF88
  units: {energy: cal, pressure: bar}
  parameters:
    a1: 0.2494
    a2: -169.09
    a3: 6.4146
    a4: -27091
    c1: 16.7439
    c2: -10465
    omega: 53240
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Al(OH)2+")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/mol, J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -8.9829e+05  -9.9558e+05  -27.53  40.832  3.8498e-06
373.15  1e+07  -8.9655e+05  -9.9241e+05  -18.172  38.014  4.3457e-06
```
