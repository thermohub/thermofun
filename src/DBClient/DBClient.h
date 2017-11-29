#ifndef DBCLIENT_H
#define DBCLIENT_H

#include <string>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QDir>

// bonio includes
#ifdef addBSONIO
#include "dbedgedoc.h"
#else
#include "bsonio/dbedgedoc.h"
#endif

namespace ThermoFun {

struct Database;
struct Reaction;
struct Element;

using mapFormulaElements = std::map<Element, double>;

///
/// \brief The DBClient class is used for comunicating with the database and retrieving data using queries
///
class DBClient
{
    boost::shared_ptr<bsonio::TDataBase> dbconnections;

    // keys list data
    boost::shared_ptr<bsonio::TDBVertexDocument> dbvertex;
    boost::shared_ptr<bsonio::TDBEdgeDocument> dbedge;

    // Connect to DataBase
    unique_ptr<bsonio::TDBVertexDocument> reactionVertex;
    unique_ptr<bsonio::TDBVertexDocument> substanceVertex;
    unique_ptr<bsonio::TDBEdgeDocument> takesEdge;
    unique_ptr<bsonio::TDBEdgeDocument> definesEdge;
    unique_ptr<bsonio::TDBVertexDocument> elementVertex;

    std::map<std::string, bson> map_id_bson;

public:

    ///
    /// \brief DBClient creates a DBClient instance
    /// \param settingsFile path to the ThermoFun.ini file
    ///
    explicit DBClient(boost::shared_ptr<bsonio::TDataBase>& adbconnections);

    DBClient();

    /// Construct a copy of an DBClient instance
    DBClient(const DBClient& other);

    /// Assign a DBClient instance to this instance
    auto operator=(DBClient other) -> DBClient&;

    virtual ~DBClient();

    ///
    /// \brief getDatabase reads from the EJDB database substances and reactions with the same sourceTDB
    /// \param sourceTDB
    /// \return returns a ThermoFun Database structure containing the substances and reactions maps
    ///
    auto getDatabase(uint sourceTDB) -> Database;

    auto parseSubstanceFormula (std::string formula) -> mapFormulaElements;
};


}

#endif // DBCLIENT_H
