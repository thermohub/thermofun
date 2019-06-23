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

auto sourceTDB_from_indexes( std::vector<int> ndx, jsonio::JsonDom *arr ) -> void
{
    ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
    jsonio::JsonDom *object;

    for(unsigned int ii=0; ii<ndx.size(); ii++)
    {
        object = arr->appendObject( std::to_string(ii) );
        object->appendString( std::to_string(ndx[ii]), enumdef->getNamebyId(ndx[ii]) );
    }
}

auto sourceTDB_from_names(std::vector<std::string> names,  jsonio::JsonDom *arr ) -> void
{
    ThriftEnumDef *enumdef = ioSettings().Schema()->getEnum("SourceTDB");
    jsonio::JsonDom *object;

    for(unsigned int ii=0; ii<names.size(); ii++)
    {
        object = arr->appendObject( std::to_string(ii) );
        object->appendString( std::to_string(enumdef->getId(names[ii])), names[ii] );
    }
}


auto sourceTDB_from_index(int ndx) -> std::string
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
    int ndx = enumdef->getId(name);
    _sourcetdb += std::to_string(ndx);
    _sourcetdb += "\":\"";
    _sourcetdb += name;
    _sourcetdb += "\"}";
    return _sourcetdb;
}
}
