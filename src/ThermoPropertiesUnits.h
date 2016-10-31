#ifndef THERMOPROPERTIESUNITS
#define THERMOPROPERTIESUNITS

#include <map>

namespace TCorrPT {

const std::map<const std::string, const std::string> defaultPropertiesUnits =

{

 {"temperature",                    "C"             },
 {"pressure",                       "bar"           },

 {"gibbs_energy",                   "J/mol"         },
 {"enthalpy",                       "J/mol"         },
 {"entropy",                        "J/mol"         },
 {"heat_capacity_cp",               "J/mol*K"       },
 {"heat_capacity_cv",               "J/mol*K"       },
 {"volume", "J/bar"},
 {"helmholtz_energy",               "J/mol"         },
 {"internal_energy",                "J/mol"         },

 {"reaction_gibbs_energy",          "J/mol"         },
 {"reaction_helmholtz_energy",      "J/mol"         },
 {"reaction_internal_energy",       "J/mol"         },
 {"reaction_enthalpy",              "J/mol"         },
 {"reaction_entropy",               "J/mol"         },
 {"reaction_volume",                "J/bar"         },
 {"reaction_heat_capacity_cp",      "J/mol*K"       },
 {"reaction_heat_capacity_cv",      "J/mol*K"       }

};

//std::map<const std::string,std::string> propertiesUnits = defaultPropertiesUnits;

}

#endif // THERMOPROPERTIESUNITS

