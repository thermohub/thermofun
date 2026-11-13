# LogKZeroEntropy

log K with zero entropy of reaction.

Calculates log K = -dG / (RT ln 10) with dS = 0, so dH = dG, the reference value. Reads log K or dG at Tr.

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_1_term_extrap1` |
| Other names | `LogKExtrap1T1` |

## Parameters

None: the method reads only the reference values of the reaction.

Accepted and not used: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files give these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them from the reference values.

## Example

No bundled record uses this method.
