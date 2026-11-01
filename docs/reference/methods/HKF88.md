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

## Built-in constants

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
