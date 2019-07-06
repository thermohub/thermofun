#include "formulaparser.h"
#include "Exception.h"
namespace ThermoFun {

const int MAXICNAME = 10;
const char* NOISOTOPE_CLASS  ="n";
const char* CHARGE_CLASS   ="z";
const char* CHARGE_NAME   ="Zz";
const short SHORT_EMPTY_  = -32768;


void BaseParser::xblanc( std::string& str )
{
    if(str.empty())
      return;
    size_t ti = str.find_first_not_of(" \n\t\r");
    if( ti == std::string::npos )
       str = "";
    else
       str = str.substr(ti);
}


//  <elem_st_coef>  ::= <double>
void BaseParser::getReal( double& valReal, std::string& parsedstr )
{
   xblanc( parsedstr );
   if( parsedstr.empty() )
        return;

   if( isdigit( parsedstr[0] )  || parsedstr[0]=='.' || parsedstr[0]=='e'  )
   {
         std::string::size_type sz;
         valReal = stod(parsedstr,&sz);
         parsedstr = parsedstr.substr(sz);
   }
//   std::cout << parsedstr << endl;
}

// --------------------------------------------------------------

ChemicalFormulaParser::~ChemicalFormulaParser()
{}

//add component to sorted list
void ChemicalFormulaParser::icadd(  std::list<ICTERM>& itt_, ICTERM& it )
{
    int iRet;
    std::list<ICTERM>::iterator itr;
    itr = itt_.begin();

    // test for setting element before
    while( itr != itt_.end() )
    {
        iRet = ictcomp( itr, it.ick, it.val );
        if( iRet == 0 )// ic find
        {
            itr->stoich += it.stoich;
            return;
        }
        if( iRet > 0 )
            break;
        itr++;
    }
    itt_.insert( itr, ICTERM( it ) );
}

void ChemicalFormulaParser::icadd(  std::list<ICTERM>& itt_, const char *icn,
                      const char *iso, int val, double csto )
{
    ICTERM term( icn, iso, val, csto );
    icadd( itt_, term );
}

int ChemicalFormulaParser::ictcomp( std::list<ICTERM>::iterator& itr, std::string& ick, int val )
{
    if( itr->ick < ick ) return(-1);
    if( itr->ick > ick ) return(1);
    if( itr->val < val ) return(-1);
    if( itr->val > val ) return(1);
    return(0);
}

//get <formula>  ::= <fterm> | <fterm><charge>
 std::list<ICTERM> ChemicalFormulaParser::parse( const std::string& aformula )
{
  formula = aformula;
  charge = "";
  std::list<ICTERM> newtt;

  scanCharge();

  // cout << formula.c_str() << charge.c_str() << endl;
  std::string term = formula;
  scanFterm( newtt, term, '\0');

  // added charge item
  if( !charge.empty() )
       addCharge( newtt );

  // TEST OUTPUT
  //cout << "Formula " << form_str << " ZZ " << charge_str << endl;
  //   for( std::size_t ii=0; ii<ict_.GetCount(); ii++)
  //cout << ict_[ii].ick << " " << ict_[ii].val << "  " << ict_[ii].stoc << endl;
  return newtt;
}

 void ChemicalFormulaParser::scanCharge( )
 {
   size_t ti = formula.find_last_of(CHARGE_TOK );
   if( ti == std::string::npos ) // no charge token
      return;

   size_t pp = formula.find( B_VALENT, ti);
   if( pp != std::string::npos )   // no charge (this is valence)
       return;

   // get charge std::string
   charge = formula.substr(ti);
   formula = formula.substr(0,ti);
 }

 // read charge
 void ChemicalFormulaParser::addCharge( std::list<ICTERM>& tt )
 {
  double cha = 1.0;
  int sign = 1;
  double aZ = 0.0;
  std::string chan = charge;
  //char *chan = (char *)charge_str.c_str();

  switch( chan[0] )
  {
     case CHARGE_NUL:    break;
     case CHARGE_MINUS:  sign = -1;
     case CHARGE_PLUS:
                         chan = chan.substr(1);
                         getReal( cha, chan );
                         aZ = cha * sign;
                         break;
     default:            break;
  }
  icadd( tt, CHARGE_NAME, CHARGE_CLASS, 1, aZ );
 }

 //get <fterm>  ::= <icterm> | <icterm><icterm>
 //    <icterm> ::= <elem>   | <elem>< elem_st_coef>
 void ChemicalFormulaParser::scanFterm( std::list<ICTERM>& itt_, std::string& startPos, char endSimb )
 {
   //char *cur_ = startPos;
   double st_coef;
   std::list<ICTERM> elt_;
   std::list<ICTERM>::iterator ite;

   while( startPos[0] != endSimb && !startPos.empty())  // list of <elem>< elem_st_coef>
   {
       // get element
       elt_.clear();

       scanElem( elt_, startPos );

       if( !startPos.empty() )
       {
         // get elem_st_coef
         st_coef = 1.;
         getReal( st_coef, startPos );
         ite = elt_.begin();
         while( ite != elt_.end() )
         {  ite->stoich *= st_coef;
            ite++;
         }
       }

       // added elements list to top level elements
       ite = elt_.begin();
       while( ite != elt_.end() )
       {
          icadd(  itt_, *ite );
          ite++;
       }
       xblanc( startPos );
       if( startPos.empty() )
           return;
    }
 }

 //get <elem>    ::= (<fterm>) | [<fterm>] |
 //                   <isotope_mass><icsymb><valence> |
 //                   <isotope_mass><icsymb> |
 //                   <icsymb><valence> | <icsymb>
 void ChemicalFormulaParser::scanElem( std::list<ICTERM>& itt_, std::string& startPos )
 {
   xblanc( startPos );
   if( startPos.empty() )
         return;

   switch( startPos[0] )
   {

     case   LBRACKET1: startPos = startPos.substr(1);
                       scanFterm( itt_, startPos, RBRACKET1 );
                       funErrorIf(startPos[0]!=RBRACKET1, "Formula", "Must be )", __LINE__, __FILE__ );
                       startPos = startPos.substr(1);
                       break;
     case   LBRACKET2: startPos = startPos.substr(1);
                       scanFterm( itt_, startPos, RBRACKET2 );
                       funErrorIf(startPos[0]!=RBRACKET2, "Formula", "Must be ]", __LINE__, __FILE__ );
                       startPos = startPos.substr(1);
                       break;
     case   LBRACKET3: startPos = startPos.substr(1);
                       scanFterm( itt_, startPos, RBRACKET3 );
                       funErrorIf(startPos[0]!=RBRACKET3, "Formula", "Must be }", __LINE__, __FILE__ );
                       startPos = startPos.substr(1);
                       break;
     case   PSUR_L_PLUS: startPos = startPos.substr(1);
                       break;
     case   'V':      if( startPos[1] == 'a' )  // Va - ignore vacancy
                       {   startPos = startPos.substr(2);
                           break;
                       } // else goto default - other <icsymb>
     default: // <isotope_mass><icsymb><valence>
         {
           std::string isotop = std::string(NOISOTOPE_CLASS);
           std::string icName = "";
           int val = SHORT_EMPTY_;

           scanIsotope( isotop, startPos);
           scanICsymb( icName, startPos);
           scanValence( val, startPos);
           icadd( itt_, icName.c_str(), isotop.c_str(), val, 1. );
           break;
        }
   }
 }


 // get <valence>   ::= |-<integer>| \ |+<integer>| \ |<integer>|
 //  <integer>    ::= <num>
 void ChemicalFormulaParser::scanValence( int& val, std::string& cur)
 {
     xblanc( cur );
     if( cur.empty() )
         return;

     if( cur[0] != B_VALENT ) // next token no valence
         return;

     cur = cur.substr(1);
     if(cur.empty())
         funError("Valence", "Term valence scan error", __LINE__, __FILE__ );

     size_t ti = cur.find_first_of(B_VALENT);
     if( ti >= 3 || ti==std::string::npos )
         funError("Valence", "Term valence scan error", __LINE__, __FILE__ );

     if( !sscanf( cur.c_str(), " %d", &val ))
         funError("Valence","Integer number scan error", __LINE__, __FILE__ );
     cur = cur.substr(ti+1);
 }

 // /3/H2/18/O             isotopic form of water.
 //  get <isotope_mass>  ::= /<integer>/
 void ChemicalFormulaParser::scanIsotope( std::string& isotop, std::string& cur)
 {
     xblanc( cur );
     if( cur.empty() )
         return;

     if( cur[0] != B_ISOTOPE ) // next token no isotop
         return;

     cur = cur.substr(1);
     if(cur.empty())
         funError("Isotope","Term isotope scan error", __LINE__, __FILE__ );

     size_t ti = cur.find_first_of(B_ISOTOPE);
     if( ti >= MAXICNAME || ti==std::string::npos )
         funError("Isotope","Term isotope scan error", __LINE__, __FILE__ );

     isotop = std::string( cur, 0, ti );  // test please
     cur = cur.substr(ti+1);
 }

 // <icsymb>    ::= <Capital_letter> \ <icsymb><lcase_letter> \ <icsymb>_
 void ChemicalFormulaParser::scanICsymb( std::string& icName, std::string& cur)
 {
     std::size_t i=1;

     xblanc( cur );
     if( cur.empty() )
         return;

     if( !iscapl( cur[0] ))
         funError("Fromula Parser"," A symbol of element expected here!", __LINE__, __FILE__ );

     for( i=1; i<=MAXICNAME+2; i++ )
        if( !islowl( cur[i]))
            break;
     funErrorIf(i>=MAXICNAME, "Fromula Parser","IC Symbol scan error", __LINE__, __FILE__ );

     icName = std::string( cur, 0, i ); //  strncpy( ic, aFa.cur, len );
     cur = cur.substr(i);
 }

 //------------------------------------------------------------------

 MoityParser::~MoityParser()
 {}

 // <fterm>  ::= <site_term> : | <fterm> <site_term>:
 // <site_term> ::= <moiety>   | <moiety><moiety>
 // <moiety>    ::= {<elem>}   | {<elem>} <elem_st_coef> | Va
 int MoityParser::parse( const std::string& aformula, std::vector<MOITERM>&  moit_ )
 {
   formula = aformula;
   std::string cur_ = formula;
   size_t endmoi;
   std::string moiName;
   double nj;   // moiety-site occupancy.
   moit_.clear();

   nSites = 0;
   while(  !cur_.empty() )  // list of {<elem>}< elem_st_coef>
   {
      switch( cur_[0])
      { case PSUR_L_PLUS:  nSites++;
                           cur_ = cur_.substr(1);
                           continue;
        case LBRACKET3: {
                       cur_ = cur_.substr(1);
                       if( cur_.empty() )
                           funError("scanMoiety","Must be }", __LINE__, __FILE__ );
                       endmoi =  cur_.find_first_of( RBRACKET3 );
                       if( endmoi == std::string::npos )
                           funError("scanMoiety","Must be }", __LINE__, __FILE__ );
                       moiName = std::string( cur_, 0, endmoi );
                       //  moiName = std::string( cur_, 0, endmoi-1 );
                       cur_ = cur_.substr(endmoi+1);
                       nj = 1.;
                       getReal( nj, cur_ );
                       moit_.push_back( MOITERM(moiName.c_str(), nSites, nj ));
                       break;
                       }
         case 'V':  if( cur_[1] == 'a' ) // Va vacancy
                    {
                       cur_ = cur_.substr(2); //Va
                       nj = 1.;
                       getReal( nj, cur_ );
                       moit_.push_back( MOITERM("Va", nSites, nj ));
                       break;
                    }
                   // else other symbol
         default:   cur_ = cur_.substr(1);
                     break;
      }
    }
   return nSites;
  }

}
