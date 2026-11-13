# LogKVantHoff

log K of van 't Hoff.

Calculates log K = (dS - dH/T) / (R ln 10) with dH and dS that do not change with T (dCp = 0). Reads log K or dG, and dH or dS, at Tr (the third of log K, dH, dS follows from the other two); without dH and dS, log K stays constant. It is LogKPolynomial with two terms: a0 = dS / (R ln 10), a2 = -dH / (R ln 10).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_2_term_extrap` |
| Other names | `LogKExtrap2T` |

## Parameters

None. The method reads only the reference values of the reaction, and makes its terms of log K from them (see above).

Not read: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them; a file can keep them, and they change nothing.

## Example

No bundled record uses this method.
