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
