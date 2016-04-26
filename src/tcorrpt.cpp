/// \file tcorrpt.cpp
/// Implementation of the TCorrPT base class
//
// Copyright (c) 2007-2012  D.Miron, D.Kulik, S.Dmitrieva
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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GEMS3K code. If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------


#include "tcorrpt.h"



Reactions TCorrPT::getReactions() const
{
    return reactions;
}

void TCorrPT::setReactions(const Reactions &value)
{
    reactions = value;
}

Substances TCorrPT::getSubstances() const
{
    return substances;
}

void TCorrPT::setSubstances(const Substances &value)
{
    substances = value;
}

TCorrPT::TCorrPT(int Nreactions, int Nsubstances)
{
    for (int i=0; i<Nreactions; i++)
    {
        reactions.push_back(new TReactionPT);
    }

    for (int i=0; i<Nsubstances; i++)
    {
        substances.push_back(new TSubstancePT);
    }
}
