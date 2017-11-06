#include "ThermoData.h"

namespace ThermoFun {

struct ThermoDataAbstract::Impl
{
    /// Vertex name
    const string name;
    /// Default query to vertex
    const string query;
    /// Paths to fields to be extracted
    vector<string> fieldPaths;
    /// Columns headers names ( for extracted data )
    vector<string> dataHeaders;
    /// Data names which will be used throughout the code
    vector<string> dataNames;

    /// Data base connection
    boost::shared_ptr<bsonio::TDBGraph> graphdb;

    Impl(const string &name, const string &query, const vector<string> &paths, const vector<string> &headers, const vector<string> &names) :
         name(name), query(query), fieldPaths(paths), dataHeaders(headers), dataNames(names)
    { }

};

ThermoDataAbstract::ThermoDataAbstract(const string &name, const string &query, const vector<string> &paths, const vector<string> &headers, const vector<string> &names)
    : pimpl(new Impl(name, query, paths, headers, names))
{}

string ThermoDataAbstract::getName() const
{
    return pimpl->name;
}

string ThermoDataAbstract::getQuery() const
{
    return pimpl->query;
}

vector<string> ThermoDataAbstract::getFieldPaths() const
{
    return pimpl->fieldPaths;
}

void ThermoDataAbstract::setFieldPaths(const vector<string> &value)
{
    pimpl->fieldPaths = value;
}

vector<string> ThermoDataAbstract::getDataHeaders() const
{
    return pimpl->dataHeaders;
}

void ThermoDataAbstract::setDataHeaders(const vector<string> &value)
{
    pimpl->dataHeaders = value;
}

vector<string> ThermoDataAbstract::getDataNames() const
{
    return pimpl->dataNames;
}

void ThermoDataAbstract::setDataNames(const vector<string> &value)
{
    pimpl->dataNames = value;
}

boost::shared_ptr<bsonio::TDBGraph> ThermoDataAbstract::getDB() const
{
    return pimpl->graphdb;
}

void ThermoDataAbstract::setDB(const boost::shared_ptr<bsonio::TDBGraph> &value)
{
    pimpl->graphdb = value;
}

bool ThermoDataAbstract::testElementsFormula( const string& aformula,
                       const vector<ElementKey>& elements )
{
   FormulaToken parser(aformula);

   for( auto formelm: parser.getElements() )
    {
      auto itr = elements.begin();
      while( itr != elements.end() )
      {
        if( formelm.first == *itr )
         break;
       itr++;
      }
      if( itr == elements.end() )
          return false;
    }
    return true;
}

vector<string> ThermoDataAbstract::getOutVertexIds( const string& edgeLabel, const string& idInVertex )
{
    vector<string> vertexIds_;
    string vertexId_;

    // select all EdgeTakes for reaction
    string queryJson = "{'_type': 'edge', '_label': '"
                     + edgeLabel  + "', '_inV': '"
                     + idInVertex + "' }";
    vector<string> _queryFields = { "_outV"};
    vector<string> _resultData;
    pimpl->graphdb->runQuery( queryJson,  _queryFields, _resultData );

    for( auto rec: _resultData)
    {
      vertexId_ =  bsonio::extractStringField( "_outV", rec );
      vertexIds_.push_back(vertexId_);
    }
    return vertexIds_;
}

vector<string> ThermoDataAbstract::getOutVertexIds(const string& edgeLabel, const string& idInVertex,  vector<string> &edgeIds_)
{
    vector<string> vertexIds_;
    string vertexId_, edgeId_;
    edgeIds_.clear();

    // select all EdgeTakes for reaction
    string queryJson = "{'_type': 'edge', '_label': '"
                     + edgeLabel  + "', '_inV': '"
                     + idInVertex + "' }";
    vector<string> _queryFields = { "_outV", "_id"};
    vector<string> _resultData;
    pimpl->graphdb->runQuery( queryJson,  _queryFields, _resultData );

    for( auto rec: _resultData)
    {
      vertexId_ =  bsonio::extractStringField( "_outV", rec );
      vertexIds_.push_back(vertexId_);
      edgeId_ =  bsonio::extractStringField( "_id", rec );
      edgeIds_.push_back(edgeId_);
    }
    return vertexIds_;
}

}
