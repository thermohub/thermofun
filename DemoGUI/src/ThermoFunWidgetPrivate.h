#ifndef THERMOFUNWIDGETPRIVATE_H
#define THERMOFUNWIDGETPRIVATE_H

#include <QObject>
#include <QtWidgets>
#include "ThermoFunWidgetNew.h"
#include "thermomodel.h"

// ThermoFun includes
#ifdef FROM_SRC
#include "../src/Interfaces/Interface.h"
#include "../src/DBClient/ReactionData.h"
#include "../src/DBClient/ReactionSetData.h"
#include "../src/DBClient/SubstanceData.h"
#include "../src/Reaction.h"
#include "../src/Substance.h"
#endif
#ifndef FROM_SRC
#include "thermofun/Interfaces/Interface.h"
#include "thermofun/DBClient/ReactionData.h"
#include "thermofun/DBClient/ReactionSetData.h"
#include "thermofun/DBClient/SubstanceData.h"
#endif

class TPContainer;
class TPropertyContainer;
class ElementsDataContainer;

using MapLevelReaction             = map<string, ThermoFun::Reaction>;
using MapLevelSubstances           = map<string, vector<ThermoFun::Substance>>;
using MapSymbolMapLevelReaction    = map<string, MapLevelReaction>;
using MapSymbolMapLevelSubstances  = map<string, MapLevelSubstances>;

extern const int defSourcetdb;

/// ThermoFun data to calculation
struct ThermoFunData
{
    string name;    ///< Task (file) name
    string comment; ///< Task description
    string calcStatus; ///< ThermoFun colaculation status

    string schemaName;  ///< Selected schema
    jsonio::DBQueryData query;       ///< Current schema
    int sourcetdb;      ///< Current sourcetdb
    vector<ThermoFun::ElementKey> elements;
    string idReactionSet; ///< Id of reaction set,
                          ///< if not empty we load records only from ReactionSet

    string unitsT;          ///< Current units of T
    string unitsP;          ///< Current units of P

    std::vector<std::vector<double>> tppairs;

    vector<string> properties;    ///< Properties names list
    vector<string> propertyUnits; ///< Units of property
    vector<int>    propertyPrecision; ///< Units of property
    int pPrecision, tPrecision;


    // work data
    map<string,string> mapUnits = ThermoFun::defaultPropertyUnits;
    map<string, int>   mapPrecision = ThermoFun::defaultPropertyDigits;


    /// Default values to task
    ThermoFunData();

    /// Write current task to configuration file fileName
    void toJsonNode( jsonio::JsonDom *object ) const;
    /// Read current task from configuration file fileName
    void fromJsonNode( const jsonio::JsonDom *object );

    ///  Clear properies and query if schema changed
    void resetSchemaName( const string& newSchemaName );
};


class ThermoFunWidgetPrivate: public QObject
{

    Q_OBJECT

    ThermoFunWidgetNew* window;

   // Internal data
//   vector<string> _shemaNames;
   string _curSchemaName = "";
   bool isDefaultQuery = false;

   ThermoFunData _data;  ///< Current project data
//   ThermoFun::SubstanceData_ subst;  ///< Working with SubstanceVerte
//   ThermoFun::ReactionData_ react;  ///<  Working with ReactionVertex
//   ThermoFun::ReactionSetData_ rcset;  ///< working with ReactionSetVertex


   // Window data  ------------------------------------

   // keys list data
   jsonui::TMatrixTable* valuesTable = 0;
   ThermoViewModel* tableModel = 0;

   // define  ELEMENTS table model
   ElementsDataContainer *elementContainer = 0;
   jsonui::TMatrixTable *elementTable = 0;
   jsonui::TMatrixModel* elementModel = 0;

   //ThermoFun data to edit;
   TPContainer* _TPContainer = 0;
   jsonui::TMatrixTable*  _TPlistTable = 0;
   jsonui::TMatrixModel*  _TPlistModel = 0;

   TPropertyContainer* _PropertyContainer = 0;
   jsonui::TMatrixTable*  _PropertyTable = 0;
   jsonui::TMatrixModel*  _PropertyModel = 0;

   void initWindow();
   void freeInternal();
   void updateElementsModel();
   void updataSourceTDB();
   void linkChange();
   void loadModel();

   MapSymbolMapLevelReaction recordsMapLevelDefinesReaction_(ThermoFun::MapSubstSymbol_MapLevel_IdReaction mapSymbol_Level_idReact);

public slots:

    void pPChanged(int val)
    {
      _data.pPrecision = val;
    }
    void tPChanged(int val)
    {
      _data.tPrecision = val;
    }
    void TUnitsChanged(const QString& text)
    {
      _data.unitsT = text.toStdString();
    }
    void PUnitsChanged(const QString& text)
    {
      _data.unitsP = text.toStdString();
    }
    void nameChanged(const QString& text)
    {
      _data.name = text.toStdString();
    }
    void commentChanged(const QString& text)
    {
      _data.comment = text.toStdString();
    }

    void retrieveConnectedData(map<string,string> mapOfConnectedIds, vector<string> &linkedSubstSymbols,
                               vector<string> &linkedReactSymbols, vector<string> &linkedSubstClasses, vector<string> &linkedSubstIds);


public:

   ThermoFunWidgetPrivate( ThermoFunWidgetNew* awindow );

   ~ThermoFunWidgetPrivate()
   {
     freeInternal();
   }

   const ThermoFunData& data() const
   {
     return _data;
   }
   const jsonio::DBQueryDef& getQuery( ) const
   {
     return tableModel->getQuery();
   }
   const jsonio::ValuesTable&  getValues() const
   {
     return tableModel->getValues();
   }
   string getThermoPropertiesName() const
   {
       std::size_t pos = std::string("Vertex").length();
       std::string schemaName = "ThermoProperties" + _data.schemaName.substr (pos);
       return schemaName;
   }
   void updateQuery( const jsonio::DBQueryData& newquery  );
   void updateElements( int sourcetdb, const vector<ThermoFun::ElementKey>& elKeys, const string& idrcset );
   void reallocTP( int newsize );
   void updateTP( const string& unitsT, const string& unitsP,
                      const std::vector<std::vector<double>>& tppairs);
   void updateProperty( const vector<string>& properties );
   void typeChanged(const string& newSchemaName);
   void newThermoFunData( const ThermoFunData& newdata );
   void updateDBClient();

   // calc functions
   void loadSubstData( const vector<int>& selNdx,
     vector<string>& aKeyList, vector<string>& substancesSymbols,
     vector<string>& substancesClass );
   void loadReactData( const vector<int>& selNdx,
     vector<string>& aKeyList, vector<string>& reactionsSymbols );
   jsonio::ValuesTable querySolvents()
   {
     return dbclient.substData().querySolvents(_data.sourcetdb);
   }

   std::map<std::string, int>  getSubstDataIndex()
   {
       return dbclient.substData().getDataName_DataIndex();
   }

   void retrieveConnectedDataSymbols(const vector<string>& substKeys, const vector<string>& reactKeys,
                                     vector<string> &linkedSubstSymbols, vector<string> &linkedReactSymbols, vector<string> &linkedSubstClasses, vector<string> &linkedSubstIds);

   MapSymbolMapLevelReaction   recordsMapLevelDefinesReaction();
   MapSymbolMapLevelReaction   recordsMapLevelDefinesReaction(vector<string> connectedSubstIds , vector<string> connectedSubstSymbols);
//   MapSymbolMapLevelSubstances recordsMapLevelTakesSubstances();

   void setSubstanceLevel(string substSymbol, string level);
//   void setReactionLevel(string reactSymbol, string level);

   double calcData(const vector<string>& substKeys, const vector<string>& reactKeys,
     const vector<string>& substancesSymbols,  const vector<string>& reactionsSymbols,
     const string& solventSymbol, bool FormatBox, bool calcSubstFromReact, bool calcReactFromSubst , struct timeval start);

   // temporaly functions
   void resetElementsintoRecord( bool isreact, const string& aKey )
   {
     if(isreact)
      dbclient.reactData().resetRecordElements( aKey );
     else
      dbclient.reactSetData().resetRecordElements( aKey );
   }

   ThermoFun::DatabaseClient dbclient;

//   const std::shared_ptr<jsonio::TDBGraph>& getReactDB( bool isreact ) const
//   {
//       if(isreact)
//        return *new std::shared_ptr<jsonio::TDBGraph>(dbclient.reactData().getDB());
//       else
//        return *new std::shared_ptr<jsonio::TDBGraph>(dbclient.reactSetData().getDB());
//   }

};



#endif // THERMOFUNWIDGETPRIVATE_H
