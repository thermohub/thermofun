    HEADERS	 += $$TCORRPT_H/tcorrpt_global.h \
    $$TCORRPT_H/Common/ThermoScalar.hpp \
    $$TCORRPT_H/Common/ScalarTypes.hpp \
    $$PWD/Solvent/Reaktoro/WaterConstants.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzState.hpp \
    $$PWD/Solvent/Reaktoro/WaterThermoState.hpp \
    $$PWD/Solvent/Reaktoro/WaterElectroState.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateHGK.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.hpp \
    $$PWD/Solvent/Reaktoro/WaterThermoStateUtils.hpp \
    $$PWD/Solvent/Reaktoro/WaterUtils.hpp \
    $$PWD/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.hpp \
#    $$PWD/tsubstancept.h \
#    $$PWD/treactionpt.h \
#    $$PWD/reactiondataprtr.h \
#    $$PWD/reactiondatapt.h \
#    $$PWD/substancedataprtr.h \
#    $$PWD/substancedatapt.h \
#    $$PWD/tcorrmodsubstance.h \
#    $$PWD/tcorrmodreaction.h \
#$$TCORRPT_H/tcorrpt.h  \
    $$PWD/Database.h \
    $$PWD/Substance.h \
    $$PWD/Reaction.h \
    $$PWD/ThermoProperties.h \
    $$PWD/ThermoParameters.h \
    $$PWD/Thermo.h \
    $$PWD/ThermoModelsSolvent.h \
    $$PWD/ThermoModelsSubstance.h \
    $$PWD/ThermoModelsReaction.h \
#    $$PWD/SubstanceSolvent.h \
    $$PWD/Common/Exception.h \
#    $$PWD/Solvent/WaterHGK.h \
    $$PWD/Solvent/WaterHGKreaktoro.h \
    $$PWD/Solvent/WaterWP95reaktoro.h \
    $$PWD/ElectroModelsSolvent.h \
    $$PWD/Solvent/WaterJN91reaktoro.h \
    $$PWD/Solvent/WaterHGK-JNgems.h \
    $$PWD/Common/OutputWaterSteamConventionProp.h \
    $$PWD/Solute/SoluteHKFreaktoro.h \
    $$PWD/Solute/SoluteHKFgems.h \
    $$PWD/Solute/SoluteADgems.h \
    $$PWD/Solvent/WaterIdealGasWolley.h \
    $$PWD/ReadFiles.h \
    $$PWD/Common/OutputToCSV.h \
    $$PWD/Solids/SolidMurnaghanHP98.h \
    $$PWD/Solids/SolidBerman88.h \
    $$PWD/Solids/SolidBMGottschalk.h \
    $$PWD/Common/EmpiricalCpIntegration.h \
    $$PWD/Solids/SolidHPLandau.h \
    $$PWD/Gases/GasCORK.h \
    $$PWD/Gases/s_solmod_.h \
    $$PWD/Solvent/WaterZhangDuan2005.h \
    $$PWD/Solvent/WaterElectroSverjensky2014.h \
    $$PWD/TPcalculationsAPI.h \
    $$PWD/Gases/GasPRSV.h \
    $$PWD/Gases/GasCGF.h \
    $$PWD/Gases/GasSRK.h \
    $$PWD/Gases/GasPR78.h \
    $$PWD/Gases/GasSTP.h \
    $$PWD/Gases/verror.h \
    $$PWD/Solvent/WaterElectroFernandez1997.h \
    $$PWD/Interfaces/Interface.h \
    $$PWD/Interfaces/InterfaceGui.h \
    $$PWD/Interfaces/Output.h


    SOURCES	 += $$PWD/Database.cpp \
    $$TCORRPT_H/Common/ThermoScalar.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateHGK.cpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.cpp \
    $$PWD/Solvent/Reaktoro/WaterThermoStateUtils.cpp \
    $$PWD/Solvent/Reaktoro/WaterUtils.cpp \
    $$PWD/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.cpp \
#    $$PWD/tsubstancept.cpp \
#    $$PWD/treactionpt.cpp \
#    $$PWD/substancedataprtr.cpp \
#    $$PWD/reactiondatapt.cpp \
#    $$PWD/reactiondataprtr.cpp \
#    $$PWD/substancedatapt.cpp \
#    $$PWD/tcorrmodsubstance.cpp \
#    $$PWD/tcorrmodreaction.cpp \
#$$TCORRPT_CPP/tcorrpt.cpp \
    $$PWD/Substance.cpp \
    $$PWD/Reaction.cpp \
    $$PWD/Thermo.cpp \
    $$PWD/ThermoModelsSolvent.cpp \
    $$PWD/ThermoModelsReaction.cpp \
    $$PWD/ThermoModelsSubstance.cpp \
#    $$PWD/SubstanceSolvent.cpp \
    $$PWD/Common/Exception.cpp \
#    $$PWD/Solvent/WaterHGK.cpp \
    $$PWD/Solvent/WaterHGKreaktoro.cpp \
    $$PWD/Solvent/WaterWP95reaktoro.cpp \
    $$PWD/ElectroModelsSolvent.cpp \
    $$PWD/Solvent/WaterJN91reaktoro.cpp \
    $$PWD/Solvent/WaterHGK-JNgems.cpp \
    $$PWD/Common/OutputWaterSteamConventionProp.cpp \
    $$PWD/Solute/SoluteHKFreaktoro.cpp \
    $$PWD/Solute/SoluteHKFgems.cpp \
    $$PWD/Solute/SoluteADgems.cpp \
    $$PWD/Solvent/WaterIdealGasWolley.cpp \
    $$PWD/ReadFiles.cpp \
    $$PWD/Common/OutputToCSV.cpp \
    $$PWD/Solids/SolidMurnaghanHP98.cpp \
    $$PWD/Solids/SolidBMGottschalk.cpp \
    $$PWD/Solids/SolidBerman88.cpp \
    $$PWD/Common/EmpiricalCpIntegration.cpp \
    $$PWD/Solids/SolidHPLandau.cpp \
    $$PWD/Gases/GasCORK.cpp \
    $$PWD/Gases/s_solmod2_.cpp \
    $$PWD/Gases/s_solmod_.cpp \
    $$PWD/Solvent/WaterZhangDuan2005.cpp \
    $$PWD/Solvent/WaterElectroSverjensky2014.cpp \
    $$PWD/TPcalculationsAPI.cpp \
    $$PWD/Gases/GasPRSV.cpp \
    $$PWD/Gases/GasCGF.cpp \
    $$PWD/Gases/GasSRK.cpp \
    $$PWD/Gases/GasPR78.cpp \
    $$PWD/Gases/GasSTP.cpp \
    $$PWD/Solvent/WaterElectroFernandez1997.cpp \
    $$PWD/Interfaces/Interface.cpp \
    $$PWD/Interfaces/InterfaceGui.cpp \
    $$PWD/Interfaces/Output.cpp
