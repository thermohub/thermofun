/// \file tcorrpt.h
/// Declarations of TCorrPT and derived classes implementing built-in models
/// of calculating standard state thermodynamic properties at temperature (T)
/// and pressure (P)

// Copyright (C) 2016-2017  D.Miron, D.Kulik, S.Dmitrieva
// <GEMS Development Team, mailto:gems2.support@psi.ch>
//
// This file is part of the GEMS3K code for thermodynamic modelling
// by Gibbs energy minimization <http://gems.web.psi.ch/GEMS3K/>
//
// GEMS3K is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// GEMS3K is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GEMS3K code. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------
//

#include <map>
#include <string>
#include <vector>

using namespace std;

#ifndef TCORRPT_H
#define TCORRPT_H

#include "tcorrpt_global.h"
#include "tsubstancept.h"
#include "treactionpt.h"

/// map between thrift keys and GEMS symbols for the general method
/// (or equation of state for both T and P correction)
extern const std::map<int, const char *> MethodGenEoS_Thrift_TO_GEMS;
extern const std::map<int, const char *> MethodCorrT_Thrift_TO_GEMS;
extern const std::map<int, const char *> MethodCorrP_Thrift_TO_GEMS;

typedef vector<TReactionPT *>  Reactions;
typedef vector<TSubstancePT *> Substances;


class TCorrPT
{
protected:

private:
    Reactions  reactions; ///
    Substances substances;

    void TCorrModReacParPT ( int ndx, int ModCode ); // should construct the correction model for reaction ndx ??
    void TCorrModSubstParPT ( int ndx, int ModCode );
    void TCorrModCreateSubstance ( );
    void TCorrModCreateReaction ( );

    void ConnectSubstancesToReactions (); // should connect the DCOMP to REACDC, especially for the REACDC

public:

    TCorrPT (int Nreactions, int Nsubstances);
    /// constructor substance


//    int  TCorrPT_init( const char *conf_json_path );

    ///< Sets current T
    void setTemperature(const double T); // set temperature (in units of K)
    ///< setes current P
    void setPressure(const double P); // set pressure (in units of Pa)

    void calculateThermoData( );

    void TCorrFromBson( /*const char *obj, TBson2Arrays&  prar*/ ); /// reading data from BSON object, bsonio?
    void TCorrToBson(  );
    void TCorrToTxt(  );
//    void MakeTCorrPT(); ///< Initializes the models for P & T claculations

    void updateStandardGibbsEnergies();
    void updateStandardVolumes();
    void updateStandardEnthalpies();
    void updateStandardEntropies();
    void updateStandardHeatCapacities();
    void updateThermoData();


    Reactions getReactions() const;
    void setReactions(const Reactions &value);
    Substances getSubstances() const;
    void setSubstances(const Substances &value);
};

#endif // TCORRPT_H
