    HEADERS	 += $$TCORRPT_H/tcorrpt_global.h \
    $$TCORRPT_H/Common/ThermoScalar.hpp \
    $$TCORRPT_H/Common/ScalarTypes.hpp \
    $$PWD/Solvent/Reaktoro/WaterConstants.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzState.hpp \
    $$PWD/Solvent/Reaktoro/WaterThermoState.hpp \
#    $$PWD/Solvent/Reaktoro/WaterElectroState.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateHGK.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.hpp \
    $$PWD/Solvent/Reaktoro/WaterThermoStateUtils.hpp \
    $$PWD/Solvent/Reaktoro/WaterUtils.hpp \
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
    $$PWD/SubstanceSolvent.h \
    $$PWD/Common/Exception.h \
#    $$PWD/Solvent/WaterHGK.h \
    $$PWD/Solvent/WaterHGKgems.h \
    $$PWD/Solvent/WaterHGKreaktoro.h


    SOURCES	 += $$PWD/Database.cpp \
    $$TCORRPT_H/Common/ThermoScalar.hpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateHGK.cpp \
    $$PWD/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.cpp \
    $$PWD/Solvent/Reaktoro/WaterThermoStateUtils.cpp \
    $$PWD/Solvent/Reaktoro/WaterUtils.cpp \
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
    $$PWD/SubstanceSolvent.cpp \
    $$PWD/Common/Exception.cpp \
#    $$PWD/Solvent/WaterHGK.cpp \
    $$PWD/Solvent/WaterHGKgems.cpp \
    $$PWD/Solvent/WaterHGKreaktoro.cpp
