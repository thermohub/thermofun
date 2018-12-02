#ifndef SOURCETDB_H
#define SOURCETDB_H

#include "jsonio/io_settings.h"

namespace ThermoFun
{
auto sourceTDB_from_index(unsigned int ndx) -> std::string;

auto sourceTDB_from_name(std::string name) -> std::string;

auto sourceTDB_from_names(std::vector<std::string> names) -> std::vector<std::string>;

auto sourceTDB_from_indexes(std::vector<int> ndx) -> std::vector<std::string>;

auto sourceTDB_from_indexes( std::vector<int> ndx, jsonio::JsonDom *arr ) -> void;

auto sourceTDB_from_names(std::vector<std::string> names,  jsonio::JsonDom *arr ) -> void;

}


#endif // SOURCETDB_H
