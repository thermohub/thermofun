#ifndef CONTAINERS_H
#define CONTAINERS_H

#include "jsonui/SelectDialog.h"
#include "thermofun/DBClient/formuladata.h"
#include "model_reactions.h"

/// Class  Elements table data container
class ElementsDataContainer : public jsonui::TAbstractDataContainer
{
    const vector<ThermoFun::ElementKey>& elements_;

public:

   ElementsDataContainer( const vector<ThermoFun::ElementKey>& elements ):
      TAbstractDataContainer("ElementsDataContainer"),
      elements_( elements )
      { resetData(); }

   virtual ~ElementsDataContainer() {}

   int rowCount() const
   { return 1;  }

   int columnCount() const
   { return elements_.size(); }

   QVariant data( int /*line*/, int column ) const
   {
     string key_ = elements_[column].key();
     return key_.c_str();
   }

   vector<string> symbolsList(  ) const
   {
     vector<string> lst;
     for (uint i = 0; i < elements_.size(); i++)
     {
         lst.push_back(elements_[i].getSymbol());
     }
     return lst;
   }

   bool setData( int /*line*/, int /*column*/, const QVariant & /*value*/ )
   {    return true;  }

   virtual QString headerData ( int ) const
   {
     return "";
   }

   virtual bool  IsEditable( int /*line*/, int /*column*/ ) const
   { return false; }

   virtual int getType( int /*line*/, int /*column*/ ) const
   { return jsonui::ftString; }

   virtual QString getToolTip( int /*line*/, int column ) const
   {
       return QString::number(column);
   }

   void resetData()
   { }

};

struct ReactData
{
    std::vector<std::string> symbols;
    std::vector<double> coeffs;

    void clear()
    {
      symbols.clear();
      coeffs.clear();
    }

    void add( const string& name, double acoef )
    {
      symbols.push_back(name);
      coeffs.push_back(std::fabs(acoef));
    }

    string reaction()
    {
       string reactR;
       for( uint jj = 0; jj <symbols.size(); jj++)
       {
         if( jj > 0 )
           reactR += " + ";
         if( coeffs[jj] != 1)
         {
             long intPart = (long) coeffs[jj];
             double fractionalPart = fabs(coeffs[jj] - intPart);
             if (fractionalPart == 0)
                 reactR += to_string( intPart );
             else
             {
                 auto str = to_string( coeffs[jj] );
                 str.erase ( str.find_last_not_of('0') + 1, std::string::npos );
                 reactR += str;
             }
         }
        reactR += symbols[jj];
       }
       return reactR;
    }

    string reaction_rev()
    {
       string reactR;
       for(size_t jj = symbols.size(); jj --> 0 ;)
       {
         if( jj < symbols.size()-1 )
           reactR += " + ";
         if( coeffs[jj] != 1)
         {
             long intPart = (long) coeffs[jj];
             double fractionalPart = fabs(coeffs[jj] - intPart);
             if (fractionalPart == 0)
                 reactR += to_string( intPart );
             else
             {
                 auto str = to_string( coeffs[jj] );
                 str.erase ( str.find_last_not_of('0') + 1, std::string::npos );
                 reactR += str;
             }
         }
        reactR += symbols[jj];
       }
       return reactR;
    }

    void buildList( ReactionTree* parent, int first, std::vector<ThermoFun::ThermoPropertiesSubstance> vTps )
    {
       ReactionTree* newChild;
       vector<double> values(8);
       vector<double> errors(8);
       vector<Reaktoro_::Status> status(8);

       for( uint jj = 0; jj <symbols.size(); jj++)
       {
          ThermoFun::ThermoPropertiesSubstance tps = vTps[jj];
          values[0] = coeffs[jj];
          if( !first )
             values[0] *= (-1);

          if (tps.gibbs_energy.sta.first != Reaktoro_::Status::notdefined)
          {values[3] = tps.gibbs_energy.val; errors[3] = tps.gibbs_energy.err;}
          status[3] = tps.gibbs_energy.sta.first;

          if (tps.enthalpy.sta.first != Reaktoro_::Status::notdefined)
          {values[4] = tps.enthalpy.val; errors[4] = tps.enthalpy.err;}
          status[4] = tps.enthalpy.sta.first;

          if (tps.entropy.sta.first != Reaktoro_::Status::notdefined)
          {values[5] = tps.entropy.val; errors[5] = tps.entropy.err;}
          status[5] = tps.entropy.sta.first;

          if (tps.heat_capacity_cp.sta.first != Reaktoro_::Status::notdefined)
          {values[6] = tps.heat_capacity_cp.val; errors[6] = tps.heat_capacity_cp.err;}
          status[6] = tps.heat_capacity_cp.sta.first;

          if (tps.volume.sta.first != Reaktoro_::Status::notdefined)
          {values[7] = tps.volume.val; errors[7] = tps.volume.err;}
          status[7] = tps.volume.sta.first;

          newChild = new ReactionTree( jj+first, symbols[jj], values, errors, status, parent );
          parent->children.push_back( unique_ptr<ReactionTree>(newChild) );
       }
    }

};

struct ISOCReactData
{
    std::vector<std::string> symbols;
    std::vector<double> coeffs;

    void clear()
    {
      symbols.clear();
      coeffs.clear();
    }

    void add( const string& name, double acoef )
    {
      symbols.push_back(name);
      coeffs.push_back(acoef);
    }

    void buildList( ReactionTree* parent, int first )
    {
       ReactionTree* newChild;
       vector<double> values(1);
       vector<double> errors(1);
       vector<Reaktoro_::Status> status(1);

       for( uint jj = 0; jj <symbols.size(); jj++)
       {
          values[0] = coeffs[jj];
          newChild = new ReactionTree( jj+first, symbols[jj], values, errors, status, parent );
          parent->children.push_back( unique_ptr<ReactionTree>(newChild) );
       }
    }

    void buildList( ReactionTree* parent, int first, std::vector<ThermoFun::ThermoPropertiesReaction> vTpr )
    {
       ReactionTree* newChild;
       vector<double> values(8);
       vector<double> errors(8);
       vector<Reaktoro_::Status> status(8);

       for( uint jj = 0; jj <symbols.size(); jj++)
       {
          ThermoFun::ThermoPropertiesReaction tpr = vTpr[jj];
          values[0] = coeffs[jj];
          if( !first )
             values[0] *= (-1);

          if (tpr.reaction_gibbs_energy.sta.first != Reaktoro_::Status::notdefined)
          {values[3] = tpr.reaction_gibbs_energy.val; errors[3] = tpr.reaction_gibbs_energy.err;}
          status[3] = tpr.reaction_gibbs_energy.sta.first;

          if (tpr.reaction_enthalpy.sta.first != Reaktoro_::Status::notdefined)
          {values[4] = tpr.reaction_enthalpy.val; errors[4] = tpr.reaction_enthalpy.err;}
          status[4] = tpr.reaction_enthalpy.sta.first;

          if (tpr.reaction_entropy.sta.first != Reaktoro_::Status::notdefined)
          {values[5] = tpr.reaction_entropy.val; errors[5] = tpr.reaction_entropy.err;}
          status[5] = tpr.reaction_entropy.sta.first;

          if (tpr.reaction_heat_capacity_cp.sta.first != Reaktoro_::Status::notdefined)
          {values[6] = tpr.reaction_heat_capacity_cp.val; errors[6] = tpr.reaction_heat_capacity_cp.err;}
          status[6] = tpr.reaction_heat_capacity_cp.sta.first;

          if (tpr.reaction_volume.sta.first != Reaktoro_::Status::notdefined)
          {values[7] = tpr.reaction_volume.val; errors[7] = tpr.reaction_volume.err;}
          status[7] = tpr.reaction_volume.sta.first;

          values[0] = coeffs[jj];
          newChild = new ReactionTree( jj+first, symbols[jj], values, errors, status, parent );
          parent->children.push_back( unique_ptr<ReactionTree>(newChild) );
       }
    }
};

#endif // CONTAINERS_H
