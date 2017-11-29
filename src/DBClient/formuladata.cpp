#include <math.h>
#include "formuladata.h"
#include "formulaparser.h"
//#include "bsonio/dbgraph.h"


DBElementsData ChemicalFormula::dbElements= DBElementsData();
vector<string> ChemicalFormula::queryFields =
{
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
          if( bsonio::is<int>( isotope, typeline.c_str()) )
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

// Writes data to bson
void ElementKey::toBson( bson *obj ) const
{
    bson_append_string( obj, "symbol", symbol.c_str() );
    if( isotope != 0 )
     bson_append_int( obj, "isotope_mass", isotope );
    if( class_ != 0 )
     bson_append_int( obj, "class_", class_ );
}

// Reads data from bson
void ElementKey::fromBson( const char* obj )
{
    if(!bsonio::bson_find_string( obj, "symbol", symbol ) )
        bsonio::bsonioErr( "ElementKey: ", "Undefined symbol.");
    if(!bsonio::bson_find_value( obj, "isotope_mass", isotope ) )
        isotope = 0;
    if(!bsonio::bson_find_value( obj, "class_", class_ ) )
        class_ = 0;
}

void ElementsToBson( bson *obj, const set<ElementKey>& elements )
{
  int ndx=0;
  for( auto el: elements)
  {
      bson_append_start_object( obj, to_string(ndx++).c_str());
      el.toBson( obj );
      bson_append_finish_object( obj );
  }
}

void ElementsToBsonArray( const char *key, bson *obj, const set<ElementKey>& elements )
{
  bson_append_start_array(obj, key);
  ElementsToBson( obj, elements );
  bson_append_finish_array( obj );
}

string ElementsToJson( const set<ElementKey>& elements )
{
  bson obj;
  bson_init( &obj );
  ElementsToBson( &obj, elements );
  bson_finish( &obj );

  string elmsjson;
  bsonio::jsonArrayFromBson( obj.data, elmsjson );
  bson_destroy( &obj );
  return elmsjson;
}


void ElementsFromBson( const char *obj, set<ElementKey>& elements )
{
    ElementKey elem("");
    bson_iterator it;
    bson_iterator_from_buffer(&it, obj );
    while (bson_iterator_next(&it))
    {
        bson_type t = bson_iterator_type(&it);
        if( t != BSON_OBJECT)
            continue;

        elem.fromBson(bson_iterator_value(&it));
        elements.insert(elem);
    }
 }

bool ElementsFromBsonArray( const char *keypath, const char *obj, set<ElementKey>& elements )
{
    elements.clear();

    bson_iterator it;
    bson_type type;
    bson_iterator_from_buffer(&it, obj );
    type =  bson_find_fieldpath_value( keypath, &it );

    if( type == BSON_ARRAY  )
    {
      ElementsFromBson( bson_iterator_value(&it), elements );
      return true;
    }
    else
      return false;
}

bool ElementsFromJson( const string elmsjson, set<ElementKey>& elements )
{
    bson obj;
    try{
        elements.clear();
        bsonio::jsonToBsonArray( &obj, elmsjson );
        ElementsFromBson( obj.data, elements );
        bson_destroy(&obj);
        return true;
    }
    catch(...)
      {
        bson_destroy(&obj);
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

        if( itr->val == bsonio::SHORT_EMPTY )
        {    auto itrdb = ChemicalFormula::getDBElements().find(key);
             if( itrdb !=  ChemicalFormula::getDBElements().end() )
                itr->val = itrdb->second.valence;
        }
        datamap.push_back( FormulaValues( key, itr->stoich, itr->val ));
        elements.insert(pair<ElementKey,double>(key,itr->stoich));
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
      if( itr->key.class_ ==  4 /*CHARGE*/ &&
          itr->valence != bsonio::SHORT_EMPTY )
         Zz += itr->stoichCoef * itr->valence;
      itr++;
    }
    return Zz;
}


void FormulaToken::clear()
{
    datamap.clear();
    elements.clear();
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
      bsonio::bsonioErr( "E37FPrun: Invalid symbol ", msg );
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
          bsonio::bsonioErr( "E37FPrun: Invalid symbol ", itr->key.symbol );

      Sc = itr->stoichCoef;
      propert.atoms_formula_unit += Sc;
      propert.atomic_mass += Sc * itrdb->second.atomic_mass;
      propert.elemental_entropy += Sc * itrdb->second.entropy;

      valence = itr->valence;
      if( valence == bsonio::SHORT_EMPTY )
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
            bsonio::bsonioErr(  "W34FPrun: Charge imbalance ", str);
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

map<ElementKey, double> ChemicalFormula::extractElements(  const vector<string>& formulalist )
{
   map<ElementKey, double> elements;
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


void ChemicalFormula::setDBElements( bsonio::TDBGraph* elementDB, const string& queryString )
{
    vector<string> resultData;
    elementDB->runQuery( queryString, queryFields, resultData );

    dbElements.clear();
    for(uint ii=0; ii<resultData.size(); ii++ )
    {
//       cout << resultData[ii] << endl;
      elementDB->SetJsonYaml(resultData[ii]);
      addOneElement( elementDB );
    }
}

void ChemicalFormula::setDBElements( bsonio::TDBGraph* elementDB, const vector<string>& keyList )
{
  dbElements.clear();

  for(uint ii=0; ii<keyList.size(); ii++ )
  {
    elementDB->GetRecord( keyList[ii].c_str() );
    addOneElement( elementDB );
  }
}

void ChemicalFormula::addOneElement( bsonio::TDBGraph* elementDB )
{
    ElementKey elkey("");
    elementDB->getValue( "properties.symbol" , elkey.symbol );
    elementDB->getValue( "properties.class_" , elkey.class_ );
    elementDB->getValue( "properties.isotope_mass" , elkey.isotope );

    ElementValues eldata;
    elementDB->getValue( "properties.atomic_mass.values.0" , eldata.atomic_mass );
    elementDB->getValue( "properties.entropy.values.0" , eldata.entropy );
    elementDB->getValue( "properties.heat_capacity.values.0" , eldata.heat_capacity );
    elementDB->getValue( "properties.volume.values.0" , eldata.volume );
    elementDB->getValue( "properties.valences.0" , eldata.valence );
    elementDB->getValue( "properties.number" , eldata.number );
    elementDB->getValue( "properties.name" , eldata.name );

    dbElements[elkey] = eldata;
}
