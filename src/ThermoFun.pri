    HEADERS	 += $$ThermoFun_H/GlobalVariables.h \
    $$ThermoFun_H/Common/ThermoScalar.hpp \
    $$ThermoFun_H/Common/ScalarTypes.hpp \
# ThermoFun
    $$PWD/Thermo.h \
    $$PWD/Database.h \
    $$PWD/Substance.h \
    $$PWD/Reaction.h \
    $$PWD/ThermoModelsSolvent.h \
    $$PWD/ElectroModelsSolvent.h \
    $$PWD/ThermoModelsSubstance.h \
    $$PWD/ThermoModelsReaction.h \
    $$PWD/ThermoProperties.h \
    $$PWD/ThermoParameters.h \
    $$PWD/ReadFiles.h \
    $$PWD/TPcalculationsAPI.h \
# Substances
    $$PWD/Substances/EmpiricalCpIntegration.h \
# Substances - Solvent
    $$PWD/Substances/Solvent/Reaktoro/WaterConstants.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterHelmholtzState.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterThermoState.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterElectroState.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterHelmholtzStateHGK.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterUtils.hpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.hpp \
    $$PWD/Substances/Solvent/WaterHGKreaktoro.h \
    $$PWD/Substances/Solvent/WaterWP95reaktoro.h \
    $$PWD/Substances/Solvent/WaterJN91reaktoro.h \
    $$PWD/Substances/Solvent/WaterHGK-JNgems.h \
    $$PWD/Substances/Solvent/WaterIdealGasWolley.h \
    $$PWD/Substances/Solvent/WaterZhangDuan2005.h \
    $$PWD/Substances/Solvent/WaterElectroSverjensky2014.h \
    $$PWD/Substances/Solvent/WaterElectroFernandez1997.h \
# Substances - Solute
    $$PWD/Substances/Solute/SoluteHKFreaktoro.h \
    $$PWD/Substances/Solute/SoluteHKFgems.h \
    $$PWD/Substances/Solute/SoluteADgems.h \
# Substances - Soldis
    $$PWD/Substances/Solids/SolidMurnaghanHP98.h \
    $$PWD/Substances/Solids/SolidBerman88.h \
    $$PWD/Substances/Solids/SolidBMGottschalk.h \
    $$PWD/Substances/Solids/SolidHPLandau.h \
# Substances - Gases
    $$PWD/Substances/Gases/GasCORK.h \
    $$PWD/Substances/Gases/GasPRSV.h \
    $$PWD/Substances/Gases/GasCGF.h \
    $$PWD/Substances/Gases/GasSRK.h \
    $$PWD/Substances/Gases/GasPR78.h \
    $$PWD/Substances/Gases/GasSTP.h \
    $$PWD/Substances/Gases/verror.h \
    $$PWD/Substances/Gases/s_solmod_.h \
# Reactions
    $$PWD/Reactions/FrantzMarshall.h \
    $$PWD/Reactions/RyzhenkoBryzgalyn.h \
    $$PWD/Reactions/LogK_function_of_T.h \
    $$PWD/Reactions/Volume_function_of_T.h \
# Common
    $$PWD/Common/Exception.h \
    $$PWD/Common/OutputWaterSteamConventionProp.h \
    $$PWD/Common/OutputToCSV.h \
# Interface
    $$PWD/Interfaces/Interface.h \
    $$PWD/Interfaces/Output.h \
# DBClient
    $$PWD/DBClient/DBClient.h \
    $$PWD/DBClient/Traversal.h

    SOURCES	 += $$PWD/Database.cpp \
    $$ThermoFun_H/Common/ThermoScalar.hpp \
# ThermoFun
    $$PWD/Thermo.cpp \
    $$PWD/Substance.cpp \
    $$PWD/Reaction.cpp \
    $$PWD/ThermoModelsSolvent.cpp \
    $$PWD/ElectroModelsSolvent.cpp \
    $$PWD/ThermoModelsReaction.cpp \
    $$PWD/ThermoModelsSubstance.cpp \
    $$PWD/TPcalculationsAPI.cpp \
    $$PWD/ReadFiles.cpp \
# Substances
    $$PWD/Substances/EmpiricalCpIntegration.cpp \
# Substances - Solvent
    $$PWD/Substances/Solvent/Reaktoro/WaterHelmholtzStateHGK.cpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.cpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterThermoStateUtils.cpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterUtils.cpp \
    $$PWD/Substances/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.cpp \
    $$PWD/Substances/Solvent/WaterHGKreaktoro.cpp \
    $$PWD/Substances/Solvent/WaterWP95reaktoro.cpp \
    $$PWD/Substances/Solvent/WaterJN91reaktoro.cpp \
    $$PWD/Substances/Solvent/WaterHGK-JNgems.cpp \
    $$PWD/Substances/Solvent/WaterIdealGasWolley.cpp \
    $$PWD/Substances/Solvent/WaterZhangDuan2005.cpp \
    $$PWD/Substances/Solvent/WaterElectroSverjensky2014.cpp \
    $$PWD/Substances/Solvent/WaterElectroFernandez1997.cpp \
# Substances - Solute
    $$PWD/Substances/Solute/SoluteHKFreaktoro.cpp \
    $$PWD/Substances/Solute/SoluteHKFgems.cpp \
    $$PWD/Substances/Solute/SoluteADgems.cpp \
# Substances - Solids
    $$PWD/Substances/Solids/SolidMurnaghanHP98.cpp \
    $$PWD/Substances/Solids/SolidBMGottschalk.cpp \
    $$PWD/Substances/Solids/SolidBerman88.cpp \
    $$PWD/Substances/Solids/SolidHPLandau.cpp \
# Substances - Gases
    $$PWD/Substances/Gases/GasCORK.cpp \
    $$PWD/Substances/Gases/GasPRSV.cpp \
    $$PWD/Substances/Gases/GasCGF.cpp \
    $$PWD/Substances/Gases/GasSRK.cpp \
    $$PWD/Substances/Gases/GasPR78.cpp \
    $$PWD/Substances/Gases/GasSTP.cpp \
    $$PWD/Substances/Gases/s_solmod2_.cpp \
    $$PWD/Substances/Gases/s_solmod_.cpp \
# Reactions
    $$PWD/Reactions/FrantzMarshall.cpp \
    $$PWD/Reactions/RyzhenkoBryzgalyn.cpp \
    $$PWD/Reactions/LogK_function_of_T.cpp \
    $$PWD/Reactions/Volume_function_of_T.cpp \
# Common
    $$PWD/Common/Exception.cpp \
    $$PWD/Common/OutputWaterSteamConventionProp.cpp \
    $$PWD/Common/OutputToCSV.cpp \
# Interface
    $$PWD/Interfaces/Interface.cpp \
    $$PWD/Interfaces/Output.cpp \
# DBClient
    $$PWD/DBClient/DBClient.cpp \
    $$PWD/DBClient/Traversal.cpp
