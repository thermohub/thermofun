# LogKZeroEntropy

log K with zero entropy of reaction.

Calculates log K = -dG / (RT ln 10) with dS = 0, so dH = dG, the reference value. Reads log K or dG at Tr. It is LogKPolynomial with one term: a2 = -dG(Tr) / (R ln 10).

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

None. The method reads only the reference values of the reaction, and makes its terms of log K from them (see above).

Not read: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them; a file can keep them, and they change nothing.

## Example

No bundled record uses this method.
