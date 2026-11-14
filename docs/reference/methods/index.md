# Methods

Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`.

## Reference properties and heat capacity

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [ConstantCp](ConstantCp.md) | Constant heat capacity | base | any |  |
| [ConstantProperties](ConstantProperties.md) | Constant properties | base | any |  |
| [ConstantVolume](ConstantVolume.md) | Constant molar volume | correction | any |  |
| [CpExtendedUNIQUAC](CpExtendedUNIQUAC.md) | Heat capacity of Extended UNIQUAC | base | aqueous_solute |  |
| [CpFeiSaxena86](CpFeiSaxena86.md) | Heat capacity of Fei and Saxena | base | component |  |
| [CpPolynomial](CpPolynomial.md) | Heat capacity polynomial | base | any |  |
| [NASA9](NASA9.md) | NASA polynomials | base | any |  |
| [TPTable](TPTable.md) | Table of T and P | base | any |  |

## Aqueous solutes

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [AkinfievDiamond03](AkinfievDiamond03.md) | Model of Akinfiev and Diamond | correction | aqueous_solute | gems |
| [AndersonEtAl91](AndersonEtAl91.md) | Aqueous solute of Anderson et al. | base | aqueous_solute |  |
| [HKF88](HKF88.md) | Revised HKF equation of state | base | aqueous_solute | reaktoro, gems |
| [SoluteHP98](SoluteHP98.md) | Aqueous solute of Holland and Powell | base | aqueous_solute | consistent, published |

## Minerals

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [BirchMurnaghan47](BirchMurnaghan47.md) | Birch-Murnaghan equation of state | correction | component | consistent, gems |
| [LambdaBerman88](LambdaBerman88.md) | Lambda transition of Berman | correction | component |  |
| [LandauHP11](LandauHP11.md) | Landau transition of Holland and Powell (2011) | correction | component |  |
| [LandauHP98](LandauHP98.md) | Landau transition of Holland and Powell | correction | component | consistent, gems |
| [MurnaghanHP98](MurnaghanHP98.md) | Murnaghan equation of Holland and Powell | correction | component | consistent, gems |
| [TaitHP11](TaitHP11.md) | Tait equation of Holland and Powell | correction | component |  |
| [VolumeBerman88](VolumeBerman88.md) | Molar volume of Berman | correction | component |  |
| [VolumeDorogokupets88](VolumeDorogokupets88.md) | Molar volume of Dorogokupets et al. | correction | component |  |

## Gases and fluids

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [ChurakovGottschalk03](ChurakovGottschalk03.md) | Equation of state of Churakov and Gottschalk | correction | gas_fluid | consistent, gems |
| [CorkHP91](CorkHP91.md) | Compensated Redlich-Kwong equation of state | correction | gas_fluid | consistent, gems |
| [IdealGas](IdealGas.md) | Ideal gas | correction | gas_fluid, component |  |
| [PRSV86](PRSV86.md) | Peng-Robinson-Stryjek-Vera equation of state | correction | gas_fluid | consistent, gems |
| [PengRobinson78](PengRobinson78.md) | Peng-Robinson equation of state | correction | gas_fluid | consistent, gems |
| [SoaveRedlichKwong72](SoaveRedlichKwong72.md) | Soave-Redlich-Kwong equation of state | correction | gas_fluid | consistent, gems |
| [SternerPitzer94](SternerPitzer94.md) | Equation of state of Sterner and Pitzer | correction | gas_fluid | consistent, gems |

## Water

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [WaterHGK84](WaterHGK84.md) | Water, Haar-Gallagher-Kell | base | water_solvent, gas_fluid | reaktoro, gems |
| [WaterIAPWS95](WaterIAPWS95.md) | Water, IAPWS-95 | base | water_solvent, gas_fluid | reaktoro |
| [WaterZhangDuan05](WaterZhangDuan05.md) | Water, Zhang and Duan | base | water_solvent |  |

## Dielectric constant of water

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [BradleyPitzer79](BradleyPitzer79.md) | Dielectric constant of Bradley and Pitzer | correction | water_solvent |  |
| [FernandezEtAl97](FernandezEtAl97.md) | Dielectric constant of Fernandez et al. | correction | water_solvent |  |
| [HelgesonKirkham74](HelgesonKirkham74.md) | Dielectric constant of Helgeson and Kirkham | correction | water_solvent |  |
| [JohnsonNorton91](JohnsonNorton91.md) | Dielectric constant of Johnson and Norton | correction | water_solvent | reaktoro, gems |
| [SverjenskyEtAl14](SverjenskyEtAl14.md) | Dielectric constant of Sverjensky et al. | correction | water_solvent |  |

## Reactions: log K

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [ConstantReactionProperties](ConstantReactionProperties.md) | Constant reaction properties | base | any |  |
| [DolejsManning10](DolejsManning10.md) | Density model of Dolejs and Manning | base | any |  |
| [LogKConstant](LogKConstant.md) | Constant log K | base | any |  |
| [LogKConstantCp](LogKConstantCp.md) | log K with constant heat capacity of reaction | base | any |  |
| [LogKPolynomial](LogKPolynomial.md) | log K polynomial | base | any |  |
| [LogKTable](LogKTable.md) | Table of log K | base | any |  |
| [LogKVantHoff](LogKVantHoff.md) | log K of van 't Hoff | base | any |  |
| [LogKZeroEntropy](LogKZeroEntropy.md) | log K with zero entropy of reaction | base | any |  |
| [MarshallFranck81](MarshallFranck81.md) | Ion product of water of Marshall and Franck | base | any |  |
| [RyzhenkoBryzgalin](RyzhenkoBryzgalin.md) | Model of Ryzhenko and Bryzgalin | base | any |  |

## Reactions: heat capacity and volume

| Method | Title | Kind | Classes | Implementations |
|---|---|---|---|---|
| [ReactionConstantVolume](ReactionConstantVolume.md) | Constant volume of reaction | correction | any |  |
| [ReactionCpPolynomial](ReactionCpPolynomial.md) | Heat capacity of reaction | correction | any |  |
| [ReactionVolumePolynomial](ReactionVolumePolynomial.md) | Volume of reaction, T and P | correction | any |  |

```{toctree}
:hidden:

reference-properties-and-heat-capacity
aqueous-solutes
minerals
gases-and-fluids
water
dielectric-constant-of-water
reactions-log-k
reactions-heat-capacity-and-volume
```
