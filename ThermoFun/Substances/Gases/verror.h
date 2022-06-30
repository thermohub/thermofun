//-------------------------------------------------------------------
// $Id: verror.h 771 2012-12-13 13:07:43Z kulik $
/// \file verror.h
/// Declarations of classes TError and TFatalError for error handling.
//
// Copyright (C) 1996-2012 A.Rysin, S.Dmytriyeva
// <GEMS Development Team, mailto:gems2.support@psi.ch>
//
// This file is part of the GEMS4K code for thermodynamic modelling
// by Gibbs energy minimization <http://gems.web.psi.ch/GEMS4K/>
//
// GEMS4K is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// GEMS4K is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GEMS4K code. If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------
#ifndef _verror_h_
#define _verror_h_

#include <string>

static const size_t npos = std::string::npos;
//   static const size_t npos = static_cast<size_t>(-1);
//   static  const size_t npos=32767;   /wp sergey 2004 from below assignment

void strip(std::string& str);


struct TError
{
    std::string mess;
    std::string title;
    TError()
    {}

    TError(const std::string& titl, const std::string& msg):
            mess(msg),
            title(titl)
    {}

    virtual ~TError()
    {}

};


struct TFatalError:
            public TError
{
    TFatalError()
    {}

    TFatalError(TError& err):
            TError(err)
    {}

    TFatalError(const std::string& titl, const std::string& msg):
            TError( titl, msg )
    {}

};


inline
void Error(const std::string& title, const std::string& message)
{
    throw TError(title, message);
}

inline
void ErrorIf(bool error, const std::string& title, const std::string& message)
{
    if(error)
        throw TError(title, message);
}


#endif
// _verror_h

