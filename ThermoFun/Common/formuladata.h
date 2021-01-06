#ifndef FORMULADATA_H
#define FORMULADATA_H

#include <iostream>
#include <map>
#include <set>
#include "formulaparser.h"

namespace ThermoFun {

class Element;

const std::map<std::string, int> elements_valences = {
    {"Ac",	3},
    {"Ag",	1},
    {"Al",	3},
    {"Ar",	0},
    {"Am",	3},
    {"As",	5},
    {"Au",	1},
    {"B",	3},
    {"Ba",	2},
    {"Be",	2},
    {"Bi",	3},
    {"Br",	-1},
    {"C",	4},
    {"Ca",	2},
    {"Cd",	2},
    {"Ce",	3},
    {"Cf",	3},
    {"Cit",	-3},
    {"Cl",	-1},
    {"Co",	2},
    {"Cr",	3},
    {"Cm",	3},
    {"Cs",	1},
    {"Cu",	2},
    {"Dy",	3},
    {"Edta",	-4},
    {"Er",	3},
    {"Eu",	3},
    {"F",	-1},
    {"Fr",  1},
    {"Fe",	2},
    {"Ga",	3},
    {"Gd",	3},
    {"Ge",	4},
    {"H",	1},
    {"He",	0},
    {"Hf",	4},
    {"Hg",	2},
    {"Ho",	3},
    {"I",	-1},
    {"In",	3},
    {"Isa",	-4},
    {"Ir",	4},
    {"K",	1},
    {"Kr",	0},
    {"La",	3},
    {"Li",	1},
    {"Lu",	3},
    {"Mg",	2},
    {"Mn",	2},
    {"Mo",	6},
    {"N",	5},
    {"N_atm",	0},
    {"Na",	1},
    {"Nb",	5},
    {"Nd",	3},
    {"Ne",	0},
    {"Ni",	2},
    {"Np",	6},
    {"O",	-2},
    {"Os",	4},
    {"Ox",	-2},
    {"P",	5},
    {"Pa",	5},
    {"Pb",	2},
    {"Pd",	2},
    {"Po",	4},
    {"Pu",	6},
    {"Pr",	3},
    {"Pm",	3},
    {"Pt",	2},
    {"Ra",	2},
    {"Rb",	1},
    {"Re",	4},
    {"Rh",	2},
    {"Rn",	0},
    {"Ru",	2},
    {"S",	6},
    {"Sb",	3},
    {"Sc",	3},
    {"Se",	4},
    {"Si",	4},
    {"Sm",	3},
    {"Sn",	2},
    {"Sr",	2},
    {"Ta",	5},
    {"Tb",	3},
    {"Tc",	7},
    {"Te",	6},
    {"Th",	4},
    {"Ti",	4},
    {"Tl",	1},
    {"Tm",	3},
    {"U",	6},
    {"V",	5},
    {"W",	6},
    {"Xe",	0},
    {"Y",	3},
    {"Yb",	3},
    {"Zn",	2},
    {"Zr",	4},
    {"Zz",	0}
};


/// Key fields of Element vertex
class ElementKey
{

  std::string symbol;
  int class_;
  int isotope;

  void classIsotopeFrom(const std::string& line );

public:

//  static int index_from_map(std::string map);

  /// Construct key from elements JSON document string
  ElementKey( const std::string& element )
  {
      fromElementNode(element);
  }

  /// Construct key from elements document fields values
  ElementKey( const std::string& asymbol, const std::string& aclass_, const std::string& aisotope  );

  /// Constructor
  ElementKey( const std::string& asymbol, int aclass /*ELEMENT*/, int aisotope ):
   symbol(asymbol), class_(aclass), isotope(aisotope)
  { }

  /// Construct key from formula parser
  ElementKey( const std::string& asymbol, const std::string& typeline ):
      symbol(asymbol)
  { classIsotopeFrom(typeline); }


  const std::string& Symbol() const
  {
    return symbol;
  }

  int Class() const
  {
    return class_;
  }

  int Isotope() const
  {
    return isotope;
  }

  /// Generate subset of formula
  std::string formulaKey() const;

  /// ElementKey from element record
  void fromElementNode( const std::string& element );

  friend bool operator <( const ElementKey&,  const ElementKey& );
  friend bool operator >( const ElementKey&,  const ElementKey& );
  friend bool operator==( const ElementKey&,  const ElementKey& );
  friend bool operator!=( const ElementKey&,  const ElementKey& );

};

auto elementKeyToElement(ElementKey elementKey) -> Element;

/// Elements values, loaded from Database
struct ElementValues
{
  std::string recid;            // Record id
  double atomic_mass;   // "Atomic (molar) mass, g/atom (g/mole)"
  double entropy;   // "Atomic entropy S0 at standard state, J/mole/K"
  double heat_capacity;   // "Atomic heat capacity Cp0 at standard state, J/mole/K"
  double volume;   // "Atomic volume V0 at standard state, cm3/mole"
  int valence;   // "Default valence number (charge) in compounds"
  int number;     // "Index in Periodical (Mendeleev's) table"
  std::string name;
  // ...
};

///     Loading from Database a map of element symbol,
/// valence, entropy and other
using DBElementsData = std::map<ElementKey, ElementValues>;

/// Values extracted from chemical formulae
struct FormulaValues
{
    ElementKey key;
    int   valence;
    double  stoichCoef;

    FormulaValues( const ElementKey& akey, double  astoichCoef, int  avalence ):
      key(akey), valence(avalence), stoichCoef(astoichCoef)
    { }
};

/// Values calculated from formula
/// vectors of molar masses, entropies and charges of substances.
struct FormulaProperites
{
    std::string formula;
    double charge;
    double atomic_mass;
    double elemental_entropy;
    double atoms_formula_unit;
};

/// Internal parsed data
using FormulaElementsData = std::vector<FormulaValues>;

/// Description of disassembled formula token
class FormulaToken
{
    std::string formula;
    FormulaElementsData datamap;  ///< List of tokens
    std::map<ElementKey, double>  elements_map;
    std::set<ElementKey>  elements;    ///< Set of existed elements
//    std::map<std::string, double> mapElementsCoeff; ///< Map of elements symbols and coefficients
    double aZ;                    ///< Calculated charge in Mol

protected:

    void clear();
    void unpack( std::list<ICTERM>& itt_ );

public:

    FormulaToken( const std::string& aformula ):aZ(0)
    {
      setFormula(aformula);
    }

    ~FormulaToken(){}

    //--- Selectors

    void setFormula( const std::string& aformula );
    const std::string& getFormula() const
    {
      return formula;
    }

    double calculateCharge();
    double charge() const
    {
        return aZ;
    }

    //--- Value manipulation

    /// Return true if all elements from formula present into system
    bool checkElements( const std::string& aformula );
    /// Throw exeption if in formula have element not present into system
    void exeptionCheckElements( const std::string& subreacKey, const std::string& aformula );
    /// Build list of elements not present into system
    std::string testElements( const std::string& aformula );

    /// Calculate charge, molar mass, elemental entropy, atoms per formula unit
    /// from chemical formulae
    void calcFormulaProperites( FormulaProperites& propert );

    /// Get a row of stoichiometry matrix from the unpacked formula,
    /// sysElemens - list of element keys
    std::vector<double> makeStoichiometryRowOld(const std::vector<ElementKey>& sysElemens );
//    Eigen::VectorXd makeStoichiometryRow(const vector<ElementKey>& sysElemens );

   /// Throw exeption if charge imbalance
    void exeptionCargeImbalance();

    /// Get of existed elements
    const std::map<ElementKey, double>& getElements_map() const
    { return elements_map; }
    const std::set<ElementKey>& getElements() const
    { return elements; }

//    const std::vector<double>& getCoefficients() const
//    { return coefficients; }

};

class ChemicalFormula
{

  /// Loading from database elements
  static  DBElementsData dbElements;
  static  std::vector<std::string> queryFields;
  static void addOneElement(Element element);

 public:

  static ElementValues undefinedProperites;

  static const DBElementsData& getDBElements()
  {
     return dbElements;
  }

  static void setDBElements(std::map<std::string, Element> elements );
  static std::vector<ElementKey> elementsRow();
  static std::map<ElementKey, double> extractElements_map(  const std::vector<std::string>& formulalist );
  static std::set<ElementKey>         extractElements(  const std::vector<std::string>& formulalist );
  static FormulaProperites         calcThermo(  const std::string formula_ );
  static std::vector<FormulaProperites> calcThermo(  const std::vector<std::string>& formulalist );
  static std::vector<std::vector<double>> calcStoichiometryMatrixOld(  const std::vector<std::string>& formulalist );
//  static Eigen::MatrixXd calcStoichiometryMatrix(  const vector<string>& formulalist );

  static const ElementValues& elementProperites( const ElementKey& elementKey )
  {
    auto itr = dbElements.find(elementKey);
    if( itr == dbElements.end() )
      return undefinedProperites;
    else
      return itr->second;
  }

};

}

#endif // FORMULADATA_H
