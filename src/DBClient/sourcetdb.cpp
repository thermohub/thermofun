#include "sourcetdb.h"

using namespace jsonio;

namespace ThermoFun
{

auto sourceTDB_from_indexes(std::vector<int> ndx) -> std::vector<std::string>
{
    std::vector<std::string> sourcetdbs;
    for (auto x : ndx)
        sourcetdbs.push_back(sourceTDB_from_index(x));
    return  sourcetdbs;
}

auto sourceTDB_from_names(std::vector<std::string> names) -> std::vector<std::string>
{
    std::vector<std::string> sourcetdbs;
    for (auto n : names)
        sourcetdbs.push_back(sourceTDB_from_name(n));
    return  sourcetdbs;
}


auto sourceTDB_from_index(uint ndx) -> std::string
{
    std::string _sourcetdb = "{\"";
    _sourcetdb += std::to_string(ndx);
    _sourcetdb += "\":\"";
    ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
    _sourcetdb += enumdef->getNamebyId(ndx);
    _sourcetdb += "\"}";
    return _sourcetdb;
}

auto sourceTDB_from_name(std::string name) -> std::string
{
    ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
    std::string _sourcetdb = "{\"";
    uint ndx = enumdef->getId(name);
    _sourcetdb += std::to_string(ndx);
    _sourcetdb += "\":\"";
    _sourcetdb += name;
    _sourcetdb += "\"}";
    return _sourcetdb;
}
}
