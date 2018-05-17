#include <math.h>
#include "jsonio/jsondomfree.h"
#include "formuladata.h"
#include "formulaparser.h"
#include "Element.h"

namespace ThermoFun {
using ElementsMap   = std::map<std::string, Element>;

DBElementsData ChemicalFormula::dbElements= DBElementsData();
vector<string> ChemicalFormula::queryFields =
{
      "_id",
      "_type",
      "_label",
      "properties.symbol",
      "properties.class_" ,
      "properties.isotope_mass",
      "properties.atomic_mass.values.0",
      "properties.entropy.values.0",
      "properties.heat_capacity.values.0",
      "properties.volume.values.0",
      "properties.valences.0",
      "properties.number",
      "properties.name"
};

void ElementKey::classIsotopeFrom(const string& typeline)
{
  class_ = 0;  // ElementClass::ELEMENT schema.enumdef->getId( "ELEMENT" );
  isotope = 0;

  if( typeline == CHARGE_CLASS )
      class_ = 4; // ElementClass::CHARGE schema.enumdef->getId( "CHARGE" );
  else
      if( typeline == NOISOTOPE_CLASS || typeline == "v" )
      {

      }
      else
          if( jsonio::is<int>( isotope, typeline.c_str()) )
             class_ = 1; // ElementClass::ISOTOPE schema.enumdef->getId( "ISOTOPE" );
          else
             isotope = 0;
  //cout << typeline << " class_ " << class_ << " isotope " << isotope << endl;
}

string ElementKey::key() const
{
  string _key= symbol;
  if( class_ != 0 )
   _key+= "|" + to_string(class_);
  if( isotope != 0 )
   _key+= "|" + to_string(isotope);

 return _key;
}

string ElementKey::getSymbol() const
{
  string _key= symbol;
  return _key;
}

// Writes data to json
void ElementKey::toJsonNode( jsonio::JsonDom *object ) const
{
    object->appendString( "symbol", symbol );
    if( isotope != 0 )
      object->appendInt( "isotope_mass", isotope );
    if( class_ != 0 )
      object->appendInt( "class_", class_ );
}

// Reads data from JsonDom
void ElementKey::fromJsonNode( const jsonio::JsonDom *object )
{
    if(!object->findValue( "symbol", symbol ) )
        jsonio::jsonioErr( "ElementKey: ", "Undefined symbol.");
    if(!object->findValue( "isotope_mass", isotope ) )
        isotope = 0;
    if(!object->findValue(  "class_", class_ ) )
        class_ = 0;
}

void ElementsToJsonDom( jsonio::JsonDom *object, const set<ElementKey>& elements )
{
  int ndx=0;
  for( auto el: elements)
  {
      auto objel = object->appendObject(to_string(ndx++));
      el.toJsonNode( objel );
  }
}

shared_ptr<jsonio::JsonDomFree> ElementsToJsonArray( const set<ElementKey>& elements )
{
    shared_ptr<jsonio::JsonDomFree> domdata(jsonio::JsonDomFree::newArray());
    ElementsToJsonDom( domdata.get(), elements );
    return domdata;
}

string ElementsToJson( const set<ElementKey>& elements )
{
  auto domarray = ElementsToJsonArray( elements );
  string elmsjson;
  printNodeToJson( elmsjson, domarray.get() );
  return elmsjson;
}


void ElementsFromJsonDom( const jsonio::JsonDom *object, set<ElementKey>& elements )
{
    ElementKey elem("");
    int objsize = object->getChildrenCount();

    for( int ii=0; ii<objsize; ii++ )
    {
       auto childobj = object->getChild( ii);
       if( childobj->getType() != jsonio::JSON_OBJECT )
            continue;
        elem.fromJsonNode(childobj);
        elements.insert(elem);
    }
 }

bool ElementsFromJsonDomArray( const string& keypath, const jsonio::JsonDom *object, set<ElementKey>& elements )
{
    elements.clear();
    auto elmobj = object->field(keypath);
    if(elmobj==nullptr)
      return false;
    ElementsFromJsonDom( elmobj, elements );
    return true;
 }


bool ElementsFromJson( const string elmsjson, set<ElementKey>& elements )
{
    try{
        auto arrobject = jsonio::unpackJson( elmsjson );
        elements.clear();
        ElementsFromJsonDom( arrobject.get(), elements );
        return true;
    }
    catch(...)
      {
        return false;
      }
}

bool operator <( const ElementKey& iEl,  const ElementKey& iEr)
{
   if( iEl.symbol < iEr.symbol )
      return true;
    if(iEl.symbol == iEr.symbol )
    {
       if( iEl.class_ < iEr.class_ )
          return true;
       else
           if( iEl.class_ == iEr.class_ )
             return ( iEl.isotope < iEr.isotope );
    }
    return false;
}

bool operator >( const ElementKey& iEl,  const ElementKey& iEr)
{
    if( iEl.symbol > iEr.symbol )
      return true;
    if(iEl.symbol == iEr.symbol )
    {
        if( iEl.class_ > iEr.class_ )
          return true;
        else
         if( iEl.class_ == iEr.class_ )
           return ( iEl.isotope > iEr.isotope );
    }
    return false;
}

bool operator ==( const ElementKey& iEl,  const ElementKey& iEr)
{
    return (iEl.symbol == iEr.symbol) &&
           (iEl.class_ == iEr.class_) &&
           (iEl.isotope == iEr.isotope) ;
}

bool operator !=( const ElementKey& iEl,  const ElementKey& iEr)
{
    return !(iEl==iEr);
}

//-------------------------------------------------------------

void FormulaToken::setFormula( const string& aformula )
{
  clear();
  formula = aformula;
  ChemicalFormulaParser formparser;
  list<ICTERM> icterms = formparser.parse( formula );
  unpack( icterms );
  aZ = calculateCharge();
}


// unpack list of terms to data
void FormulaToken::unpack( list<ICTERM>& itt_ )
{
    list<ICTERM>::iterator itr;
    itr = itt_.begin();
    while( itr != itt_.end() )
    {
        ElementKey key( itr->ick, itr->ick_iso );

        if( itr->val == jsonio::SHORT_EMPTY )
        {    auto itrdb = ChemicalFormula::getDBElements().find(key);
             if( itrdb !=  ChemicalFormula::getDBElements().end() )
                itr->val = itrdb->second.valence;
        }
        datamap.push_back( FormulaValues( key, itr->stoich, itr->val ));
        elements.insert(key);
        elements_map.insert(pair<ElementKey,double>(key,itr->stoich));
//        coefficients.push_back(itr->stoich);
        itr++;
    }
}

double FormulaToken::calculateCharge()
{
    double  Zz=0.0;
    auto itr =  datamap.begin();
    while( itr != datamap.end() )
    {
      if( itr->key.class_ !=  4 /*CHARGE*/ &&
          itr->valence != jsonio::SHORT_EMPTY )
         Zz += itr->stoichCoef * itr->valence;
      itr++;
    }
    return Zz;
}


void FormulaToken::clear()
{
    datamap.clear();
    elements.clear();
    elements_map.clear();
    aZ = 0.;
}


bool FormulaToken::checkElements( const string& aformula )
{
  string notPresent = testElements( aformula );
  return notPresent.empty();
}

void FormulaToken::exeptionCheckElements( const string& subreacKey, const string& aformula )
{
  string notPresent = testElements( aformula );
  if( !notPresent.empty() )
  {
      string msg = "Invalid Elements: ";
      msg += notPresent;
      msg += "\n in formula in record: \n";
      msg += subreacKey;
      jsonio::jsonioErr( "E37FPrun: Invalid symbol ", msg );
  }
}

// Test all IComps in DB
string FormulaToken::testElements( const string& aformula )
{
    string notPresent = "";
    setFormula( aformula );

    auto itr = datamap.begin();
    while( itr != datamap.end() )
    {
      if( ChemicalFormula::getDBElements().find(itr->key) ==
              ChemicalFormula::getDBElements().end() )
        notPresent += itr->key.symbol + ";";
      itr++;
    }
    return notPresent;
}

void FormulaToken::calcFormulaProperites( FormulaProperites& propert )
{
    double Sc, valence;
    propert.formula = formula;
    propert.charge = propert.atomic_mass = 0.0;
    propert.elemental_entropy = propert.atoms_formula_unit = 0.0;

    auto itr = datamap.begin();
    while( itr != datamap.end() )
    {
      auto itrdb = ChemicalFormula::getDBElements().find(itr->key);
      if( itrdb ==  ChemicalFormula::getDBElements().end() )
          jsonio::jsonioErr( "E37FPrun: Invalid symbol ", itr->key.symbol );

      Sc = itr->stoichCoef;
      propert.atoms_formula_unit += Sc;
      propert.atomic_mass += Sc * itrdb->second.atomic_mass;
      propert.elemental_entropy += Sc * itrdb->second.entropy;

      valence = itr->valence;
      if( valence == jsonio::SHORT_EMPTY )
          valence = itrdb->second.valence;
      if( itr->key.class_ !=  4 /*CHARGE*/ )
          propert.charge += Sc * valence;

      itr++;
    }
}


// Get a row of stoichiometry matrix
vector<double> FormulaToken::makeStoichiometryRowOld( const vector<ElementKey>& sysElemens )
{
    double ai = 0;
    vector<double> rowA;

    auto itel = sysElemens.begin();
    while( itel != sysElemens.end() )
    {
      ai=0.;
      const ElementKey& elkey = *itel;
      if( elements.find(elkey) != elements.end() )
      {
          for(uint ii=0; ii<datamap.size(); ii++ )
              if( elkey == datamap[ii].key )
                 ai += datamap[ii].stoichCoef;
      }
      rowA.push_back(ai);
      itel++;
    }
    return rowA;
}

// Get a row of stoichiometry matrix
//Eigen::VectorXd FormulaToken::makeStoichiometryRow( const vector<ElementKey>& sysElemens )
//{
//    double ai = 0;
//    Eigen::VectorXd rowA(sysElemens.size());

//    for( uint ii=0; ii<sysElemens.size(); ii++ )
//    {
//      ai=0.;
//      const ElementKey& elkey = sysElemens[ii];
//      if( elements.find(elkey) != elements.end() )
//      {
//          for(uint ii=0; ii<datamap.size(); ii++ )
//              if( elkey == datamap[ii].key )
//                 ai += datamap[ii].stoichCoef;
//      }
//      rowA[ii] =ai;
//    }
//    return rowA;
//}


void FormulaToken::exeptionCargeImbalance()
{
  ElementKey chargeKey( CHARGE_NAME,4 );
  if( elements.find(chargeKey) == elements.end() )
    return;

  for(int ii=datamap.size()-1; ii>0; ii-- )
      if( chargeKey == datamap[ii].key )
      {
         double Zzval = datamap[ii].stoichCoef;

         if( fabs( (aZ - Zzval) ) > 1e-6 )
         {
            string str = "In the formula: ";
            str +=  formula + "\n calculated charge: ";
            str +=  to_string(aZ) + " != " + to_string(Zzval);
            //aSC[ii] = aZ;  // KD 03.01.04  - temporary workaround (adsorption)
            jsonio::jsonioErr(  "W34FPrun: Charge imbalance ", str);
         }
         break;
     }
}


//------------------------------------------

vector<ElementKey> ChemicalFormula::elementsRow()
{
    vector<ElementKey> row;
    auto itr =dbElements.begin();

    while( itr != dbElements.end() )
      row.push_back( ElementKey(itr++->first) );
    return row;
}

map<ElementKey, double> ChemicalFormula::extractElements_map(  const vector<string>& formulalist )
{
   map<ElementKey, double> elements_map;
   FormulaToken formula("");

   for(uint ii=0; ii<formulalist.size(); ii++ )
   {
     formula.setFormula(  formulalist[ii] );
     elements_map.insert( formula.getElements_map().begin(), formula.getElements_map().end());
   }
   return elements_map;
}

set<ElementKey> ChemicalFormula::extractElements(  const vector<string>& formulalist )
{
   set<ElementKey> elements;
   FormulaToken formula("");

   for(uint ii=0; ii<formulalist.size(); ii++ )
   {
     formula.setFormula(  formulalist[ii] );
     elements.insert( formula.getElements().begin(), formula.getElements().end());
   }
   return elements;
}

vector<FormulaProperites> ChemicalFormula::calcThermo(  const vector<string>& formulalist )
{
   vector<FormulaProperites> thermo;
   FormulaToken formula("");
   FormulaProperites val;

   for(uint ii=0; ii<formulalist.size(); ii++ )
   {
     formula.setFormula(  formulalist[ii] );
     formula.calcFormulaProperites( val );
     thermo.push_back( val );
   }

   return thermo;
}

FormulaProperites ChemicalFormula::calcThermo(  const string formula_ )
{
   FormulaToken formula("");
   FormulaProperites val;
   formula.setFormula(  formula_ );
   formula.calcFormulaProperites( val );
   return val;
}

vector<vector<double>> ChemicalFormula::calcStoichiometryMatrixOld(  const vector<string>& formulalist )
{
   vector<vector<double>> matrA;
   vector<ElementKey> sysElemens = elementsRow();
   FormulaToken formula("");

   for(uint ii=0; ii<formulalist.size(); ii++ )
   {
     formula.setFormula( formulalist[ii] );
     matrA.push_back( formula.makeStoichiometryRowOld( sysElemens ) );
   }

   return matrA;
}

//Eigen::MatrixXd ChemicalFormula::calcStoichiometryMatrix(  const vector<string>& formulalist )
//{
//   vector<ElementKey> sysElemens = elementsRow();
//   Eigen::MatrixXd matrA(formulalist.size(), sysElemens.size());
//   FormulaToken formula("");

//   for(uint ii=0; ii<formulalist.size(); ii++ )
//   {
//     formula.setFormula( formulalist[ii] );
//     matrA.row(ii) = formula.makeStoichiometryRow( sysElemens );
//   }

//   return matrA;
//}

void ChemicalFormula::setDBElements(ElementsMap elements )
{
    for (auto e : elements)
        addOneElement(e.second);
}

void ChemicalFormula::setDBElements( jsonio::TDBVertexDocument* elementDB, const jsonio::DBQueryData& query )
{
    vector<string> resultData;
    elementDB->runQuery( query, queryFields, resultData );

    dbElements.clear();
    for(uint ii=0; ii<resultData.size(); ii++ )
    {
//       cout << resultData[ii] << endl;
      elementDB->SetJsonYaml(resultData[ii]);
      addOneElement( elementDB );
    }
}

void ChemicalFormula::setDBElements( jsonio::TDBVertexDocument* elementDB, const vector<string>& keyList )
{
  dbElements.clear();

  for(uint ii=0; ii<keyList.size(); ii++ )
  {
    elementDB->GetRecord( keyList[ii].c_str() );
    addOneElement( elementDB );
  }
}

vector<ElementKey> getDBElements( jsonio::TDBVertexDocument* elementDB, const vector<string>& idList )
{
  vector<ElementKey> elements;
  ElementKey elkey("");

  for(uint ii=0; ii<idList.size(); ii++ )
  {
    elementDB->GetRecord( (idList[ii]+":").c_str() );
    elementDB->getValue( "properties.symbol" , elkey.symbol );
    elementDB->getValue( "properties.class_" , elkey.class_ );
    elementDB->getValue( "properties.isotope_mass" , elkey.isotope );
    elements.push_back(elkey);
  }

  return elements;
}


void ChemicalFormula::addOneElement(Element e)
{
    ElementKey elkey("");
    elkey.symbol = e.symbol();
    elkey.class_ = e.class_();
    elkey.isotope = e.isotopeMass();

    ElementValues eldata;
//    eldata.recid =;
    eldata.atomic_mass = e.molarMass();
    eldata.entropy = e.entropy();
    eldata.heat_capacity = e.heatCapacity();
//    eldata.volume =;
//    eldata.valence =;
//    eldata.number =;
    eldata.name = e.name();

    dbElements[elkey] = eldata;
}

void ChemicalFormula::addOneElement( jsonio::TDBVertexDocument* elementDB )
{
    ElementKey elkey("");
    elementDB->getValue( "properties.symbol" , elkey.symbol );
    elementDB->getValue( "properties.class_" , elkey.class_ );
    elementDB->getValue( "properties.isotope_mass" , elkey.isotope );

    ElementValues eldata;
    elementDB->getValue( "_id" , eldata.recid );
    elementDB->getValue( "properties.atomic_mass.values.0" , eldata.atomic_mass );
    elementDB->getValue( "properties.entropy.values.0" , eldata.entropy );
    elementDB->getValue( "properties.heat_capacity.values.0" , eldata.heat_capacity );
    elementDB->getValue( "properties.volume.values.0" , eldata.volume );
    elementDB->getValue( "properties.valences.0" , eldata.valence );
    elementDB->getValue( "properties.number" , eldata.number );
    elementDB->getValue( "properties.name" , eldata.name );

    dbElements[elkey] = eldata;
}

auto elementKeyToElement(ElementKey elementKey) -> Element
{
    Element e;
    auto itrdb = ChemicalFormula::getDBElements().find(elementKey);
    if (itrdb == ChemicalFormula::getDBElements().end())
        jsonio::jsonioErr("E37FPrun: Invalid symbol ", elementKey.symbol);

    e.setClass(elementKey.class_);
    e.setIsotopeMass(elementKey.isotope);
    e.setSymbol(elementKey.symbol);
    e.setName(itrdb->second.name);
    e.setMolarMass(itrdb->second.atomic_mass);
    e.setEntropy(itrdb->second.entropy);
    e.setHeatCapacity(itrdb->second.heat_capacity);
    e.setVolume(itrdb->second.volume);
    e.setValence(itrdb->second.valence);

    return e;
}


}
