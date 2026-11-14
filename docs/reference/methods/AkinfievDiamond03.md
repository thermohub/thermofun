# AkinfievDiamond03

Model of Akinfiev and Diamond.

Calculates the properties of a dissolved gas or neutral species from those of the gas and of water.

Source: Akinfiev and Diamond (2003).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | aqueous_solute |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `solute_aknifiev_diamond03` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| xi | 1 | 1 | yes |  | parameter xi |
| a | cm3/g | m3/kg | yes |  | parameter a |
| b | cm3.K^0.5/g | m3.K^0.5/kg | yes |  | parameter b |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: M: g/mol. They are not converted.

Dissolved gases and neutral species. MW: the molar mass of water in g/mol that the model uses for the volume of water and its mole number. It has other digits than the molar mass of water of the rest of ThermoFun, and is kept to reproduce the model.

Source: Akinfiev N.N., Diamond L.W. (2003) Geochim. Cosmochim. Acta 67, 613-629.

Names: `MW`.

## Example

From CH4@ in `aq17-thermofun.json`.

```yaml
- method: AkinfievDiamond03
  parameters:
    xi: -0.1131
    a: -11.8462
    b: 14.8615
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "CH4@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -34132  -86797  90.463  249.1  3.7175e-05
373.15  1e+07  -42498  -70369  138.73  198.24  3.8345e-05
```
