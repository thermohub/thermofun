# LogKConstant

Constant log K.

Keeps log K at the reference value at every T: dH = dS = dCp = 0. Reads log K or dG at Tr. It is LogKPolynomial with one term: a0 = log K(Tr).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_1_term_extrap0` |
| Other names | `LogKExtrap1T0` |

## Parameters

None. The method reads only the reference values of the reaction, and makes its terms of log K from them (see above).

Not read: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them; a file can keep them, and they change nothing.

## Example

No bundled record uses this method.
