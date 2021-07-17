#include <math.h>
#include "formuladata.h"
#include "formulaparser.h"
#include "Element.h"
#include "Exception.h"
// JSON
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace  std;

namespace ThermoFun {
using ElementsMap   = std::map<std::string, Element>;
const short SHORT_EMPTY_  = -32768;

DBElementsData ChemicalFormula::dbElements= DBElementsData();
//vector<string> ChemicalFormula::queryFields =
//{
//      "_id",
//      "_type",
//      "_label",
//      "properties.symbol",
//      "properties.class_" ,
//      "properties.isotope_mass",
//      "properties.atomic_mass.values.0",
//      "properties.entropy.values.0",
//      "properties.heat_capacity.values.0",
//      "properties.volume.values.0",
//      "properties.valences.0",
//      "properties.number",
//      "properties.name"
//};

// Construct key from elements document values
ElementKey::ElementKey(const std::string& asymbol, const string &aclass_, const std::string& aisotope  ):
    symbol(asymbol)
{
    json j = json::parse(aclass_);
    isotope = 0;
    if( !aisotope.empty() )
           isotope = stoi(aisotope);
    class_  = stoi(j.begin().key());
}

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
//          if( jsonio::is<int>( isotope, typeline.c_str()) )
//             class_ = 1; // ElementClass::ISOTOPE schema.enumdef->getId( "ISOTOPE" );
//          else
             isotope = 0;
  //cout << typeline << " class_ " << class_ << " isotope " << isotope << endl;

}

string ElementKey::formulaKey() const
{
  string _key= symbol;
  if( class_ != 0 )
   _key+= "|" + to_string(class_);
  if( isotope != 0 )
   _key+= "|" + to_string(isotope);

 return _key;
}

// Reads data from JsonDom (fromm real record)
void ElementKey::fromElementNode( const std::string& element )
{
  json j = json::parse(element);

  if (j.contains("symbol"))
    symbol = j["symbol"].get<string>();
  else
    funError("Undefined symbol.", "Element ", __LINE__, __FILE__ );
  if (j.contains("isotope_mass"))
    isotope = j["isotope_mass"].get<int>();
  else
    isotope = 0;
  if (j.contains("class_"))
    class_ = stoi(j["class_"].begin().key());
  else
    class_ = 0;
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
  formula.erase(std::remove(formula.begin(), formula.end(), '\"'), formula.end());
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

        if( itr->val == SHORT_EMPTY_ )
        {    auto itrdb = ChemicalFormula::getDBElements().find(key);
             if( itrdb !=  ChemicalFormula::getDBElements().end() )
                itr->val = itrdb->second.valence;
        }
        datamap.push_back( FormulaValues( key, itr->stoich, itr->val ));
        elements.insert(key);
        if (elements_map.find(key) != elements_map.end())
            elements_map.at(key) += itr->stoich;
        else
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
      if( itr->key.Class() !=  4 /*CHARGE*/ &&
          itr->valence != SHORT_EMPTY_ )
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
      funError("Invalid symbol", msg, __LINE__, __FILE__);
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
        notPresent += itr->key.Symbol() + ";";
      itr++;
    }
    return notPresent;
}

void FormulaToken::calcFormulaProperites( FormulaProperites& propert )
{
    double Sc;
    int valence;
    propert.formula = formula;
    propert.charge = propert.atomic_mass = 0.0;
    propert.elemental_entropy = propert.atoms_formula_unit = 0.0;

    auto itr = datamap.begin();
    while( itr != datamap.end() )
    {
      auto itrdb = ChemicalFormula::getDBElements().find(itr->key);
      if( itrdb ==  ChemicalFormula::getDBElements().end() )
          funError("Invalid symbol", itr->key.Symbol(), __LINE__, __FILE__);

      Sc = itr->stoichCoef;
      propert.atoms_formula_unit += Sc;
      propert.atomic_mass += Sc * itrdb->second.atomic_mass;
      propert.elemental_entropy += Sc * itrdb->second.entropy;

      valence = itr->valence;
      if( valence == SHORT_EMPTY_ )
          valence = itrdb->second.valence;
      if( itr->key.Class() !=  4 /*CHARGE*/ )
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
          for(unsigned int ii=0; ii<datamap.size(); ii++ )
              if( elkey == datamap[ii].key )
                 ai += datamap[ii].stoichCoef;
      }
      rowA.push_back(ai);
      itel++;
    }
    return rowA;
}

void FormulaToken::exeptionCargeImbalance()
{
    ElementKey chargeKey( CHARGE_NAME,4,0 );
    if( elements.find(chargeKey) == elements.end()  )
        return;

    auto itr =  datamap.begin();
    while( itr != datamap.end() )
    {
        if( chargeKey == itr->key )
        {
            double Zzval = itr->stoichCoef;

            if( fabs( (aZ - Zzval) ) > 1e-6 )
            {
                string str = "In the formula: ";
                str +=  formula + "\n calculated charge: ";
                str +=  to_string(aZ) + " != " + to_string(Zzval);
                //aSC[ii] = aZ;  // KD 03.01.04  - temporary workaround (adsorption)
                funError("Charge imbalance", str, __LINE__, __FILE__);
            }
            break;
        }
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

   for(unsigned int ii=0; ii<formulalist.size(); ii++ )
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

   for(unsigned int ii=0; ii<formulalist.size(); ii++ )
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

   for(unsigned int ii=0; ii<formulalist.size(); ii++ )
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

   for(unsigned int ii=0; ii<formulalist.size(); ii++ )
   {
     formula.setFormula( formulalist[ii] );
     matrA.push_back( formula.makeStoichiometryRowOld( sysElemens ) );
   }

   return matrA;
}

void ChemicalFormula::setDBElements(ElementsMap elements )
{
    for (auto e : elements)
        addOneElement(e.second);
}

void ChemicalFormula::addOneElement(Element e)
{
    ElementKey elkey(e.symbol(), e.class_(), e.isotopeMass() );
    ElementValues eldata;

//    eldata.recid =;
    eldata.atomic_mass = e.molarMass();
    eldata.entropy = e.entropy();
    eldata.heat_capacity = e.heatCapacity();
    eldata.volume = e.volume();
    if (e.valence()==777)
    {
        if (elements_valences.find(e.symbol()) != elements_valences.end())
            e.setValence(elements_valences.at(e.symbol()));
        else
            e.setValence(0);
    }
    eldata.valence = e.valence();
    eldata.number = e.number();
    eldata.name = e.symbol(); // was e.name();

    dbElements[elkey] = eldata;
}

auto elementKeyToElement(ElementKey elementKey) -> Element
{
    Element e;
    auto itrdb = ChemicalFormula::getDBElements().find(elementKey);
    if (itrdb == ChemicalFormula::getDBElements().end())
        funError("Invalid symbol", elementKey.Symbol(), __LINE__, __FILE__);

    e.setClass(elementKey.Class());
    e.setIsotopeMass(elementKey.Isotope());
    e.setSymbol(elementKey.Symbol());
    e.setName(itrdb->second.name);
    e.setMolarMass(itrdb->second.atomic_mass);
    e.setEntropy(itrdb->second.entropy);
    e.setHeatCapacity(itrdb->second.heat_capacity);
    e.setVolume(itrdb->second.volume);
    e.setValence(itrdb->second.valence);
    e.setNumber(itrdb->second.number);

    return e;
}


}
