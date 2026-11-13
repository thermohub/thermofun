# LogKVantHoff

log K of van 't Hoff.

Calculates log K = (dS - dH/T) / (R ln 10) with dH and dS that do not change with T (dCp = 0). Reads log K or dG, and dH or dS, at Tr (the third of log K, dH, dS follows from the other two); without dH and dS, log K stays constant.

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

None: the method reads only the reference values of the reaction.

Accepted and not used: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files give these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them from the reference values.

## Example

No bundled record uses this method.
