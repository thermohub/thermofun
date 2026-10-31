# IdealGas

Ideal gas.

Sets V = RT/P and adds RT ln(P/P0) to G.

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

From C12A7 in `cemdata18-thermofun.json`.

```yaml
- method: IdealGas
```
