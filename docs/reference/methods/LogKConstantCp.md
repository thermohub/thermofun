# LogKConstantCp

log K with constant heat capacity of reaction.

Calculates log K = (dS - dH/T - dCp (1 - Tr/T - ln(T/Tr))) / (R ln 10) with a dCp that does not change with T. Reads log K or dG, dH or dS, and dCp at Tr; without dCp it is van 't Hoff, without dH and dS a constant log K.

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_3_term_extrap` |
| Other names | `LogKExtrap3T` |

## Parameters

None: the method reads only the reference values of the reaction.

Accepted and not used: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files give these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them from the reference values.

## Example

From M075SH in `cemdata18-thermofun.json`: the reference values it reads, and the method.

```yaml
reaction_properties:
  logK: -28.8
  dG: 161635
  dH: 57008.6
  dS: -356.903
  dCp: -595.047
methods:
  - method: LogKConstantCp
```
