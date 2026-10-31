# ReactionConstantVolume

Constant volume of reaction.

Adds dV dP to dG and dH, and the same term to log K.

| | |
|---|---|
| Works on | reaction |
| Kind | correction |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `dr_volume_constant` |
| Other names | `DrVolumeConst` |

## Example

From M075SH in `cemdata18-thermofun.json`.

```yaml
- method: ReactionConstantVolume
```
