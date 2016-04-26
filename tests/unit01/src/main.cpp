#include <iostream>
#include "tcorrpt.h"

using namespace std;

int main()
{
    /// variables substance ///
    vd sV0,              ///< Standard volume at Tst, Pst "V0" m3/mol; [1] ERROR
       sG0,              ///< Standard Gibbs energy at Tst, Pst  "GO" J/mol; [1] ERROR
       sH0,              ///< Standard enthalpy at Tst, Pst  "H0" J/mol; [1] ERROR
       sS0,              ///< Standard entropy of formation from elements at st.states "S0f" J/K/mol; [1] ERROR
       saS0,             ///< Standard absolute entropy at Tst, Pst "S0" J/K/mol;[1] ERROR
       sCp0;             ///< Standard constant-pressure heat capacity at Tst, Pst "Cp0" J/K/mol; [1] ERROR

    vd G0, H0, S0,         ///< partial molal gibbs energy, enthalpy, entropy at current TP; [1] ERROR
       V0, Cp0,               ///< partial molal volume, heat capacity at current TP; [1] ERROR
       Fug, dVg;              ///< molar fugacity at TP (reserved), dVm of gas (reserved); [1] ERROR

    double Tr = 298.15,                  ///< Reference temperature for standard state (usually 298.15 K)
           Pr = 1e05;                  ///< Reference pressure (usually 1 bar or 10^5 Pa, sometimes 1.013 bar = 1 atm)
    double currentP = 698.15,                    ///< 400 C
           currentT = 2e08;                    ///< 2000 bar

   /// END variables substance ///

    cout << "Hello World!" << endl;


    TCorrPT Tcorr = TCorrPT(1,2);


    cout << "Bye World!" << endl;


    return 0;
}

