// C++ includes
#include <string>
#include <map>
#include <vector>
//#include <set>

namespace ThermoFun {

struct DatabaseClient;
struct Database;

using List_VertexId_VertexType = std::vector<std::pair<std::string, std::string>>;

enum DefinesSubstLevelMode {
    followAll= 0,
    followGiven,
    followDiefferentPerSubstance
};

struct DefinesSubstLevelOptions {
    /// map of substance symbol and level for the edge defines connected to the substance
    std::map<std::string, std::string> substSymbol_definesLevel;

    /// current level for the traversal operation
    std::string level = "0";

    /// mode of using levels for traversal (all: collect all connected; single: collect all connected using one
    /// defines level for all substances; multiple: collect all connectec using different levels for different
    /// substances (from map substSymbol_definesLevel)
    DefinesSubstLevelMode definesSubstLevelMode = DefinesSubstLevelMode::followAll;
};

auto databaseFromRecordList(const DatabaseClient &dbc, const List_VertexId_VertexType &recordList, DefinesSubstLevelOptions levelOptions = DefinesSubstLevelOptions()) -> Database;


}
