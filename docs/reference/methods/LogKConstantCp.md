# LogKConstantCp

log K with constant heat capacity of reaction.

Calculates log K = (dS - dH/T - dCp (1 - Tr/T - ln(T/Tr))) / (R ln 10) with a dCp that does not change with T. Reads log K or dG, dH or dS, and dCp at Tr; without dCp it is van 't Hoff, without dH and dS a constant log K. It is LogKPolynomial with three terms: a0 = (dS - dCp (1 + ln Tr)) / (R ln 10), a2 = -(dH - dCp Tr) / (R ln 10), a3 = dCp / (R ln 10).

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

None. The method reads only the reference values of the reaction, and makes its terms of log K from them (see above).

Not read: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them; a file can keep them, and they change nothing.

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
