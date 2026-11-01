# IdealGas

Ideal gas.

Sets V = RT/P, adds RT ln(P/P0) to G and subtracts R ln(P/P0) from S.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid, component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_pvnrt` |
| Other names | `IdealGasVol` |

## Example

From CH4 in `cemdata18-thermofun.json`.

```yaml
- method: IdealGas
```
