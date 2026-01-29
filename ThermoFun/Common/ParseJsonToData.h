//#include "Database.h"
#include <string>

namespace ThermoFun {

class Substance;
class Reaction;
class Element;
struct DatabaseConventions;

//DefinesSubstLevelOptions levelOptionsDefault;
/// Parses an element bson object and loads its data into the element structure
/// @param bso bson object
auto parseElement (const std::string& data) -> Element;

/// Parses a substance bson object and loads its data into the substance structure
/// @param bso bson object
auto parseSubstance (const std::string &data) -> Substance;

/// Parses a reaction bson object and loads its data into the reaction structure
/// @param bso bson object
auto parseReaction (const std::string &data) -> Reaction;

/// Parses object j["conventions"] from database 
auto readConventions(const std::string &data) -> DatabaseConventions;

}
