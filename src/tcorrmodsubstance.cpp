#include "tcorrmodsubstance.h"



//=======================================================================================================
// TCorrModSubstance base class for T and P correction of thermodynamic properties for substances
// (c) DM April 2016 (version 1)
//=======================================================================================================

///< Constructor
TCorrModSubstance::TCorrModSubstance(SubstanceDataPrTr *AtPrTr_, SubstanceDataPT *AtPT_)
{
    AtPrTr = AtPrTr_;
    AtPT = AtPT_;
}

// Getter and Setter functions
vvd TCorrModSubstance::getTCinterval() const
{
    return TCinterval;
}

void TCorrModSubstance::setTCinterval(const vvd &value)
{
    TCinterval = value;
}
vvd TCorrModSubstance::getCpCoeff() const
{
    return CpCoeff;
}

void TCorrModSubstance::setCpCoeff(const vvd &value)
{
    CpCoeff = value;
}


//=======================================================================================================
// Integration of empirical heat capacity equation Cp=f(T);
// References:
// Added: DM 26.04.2016
//=======================================================================================================

///< Constructor
EmpCpIntegr::EmpCpIntegr(SubstanceDataPrTr *AtPrTr_, SubstanceDataPT *AtPT_):
    TCorrModSubstance( AtPrTr_, AtPT_ )
{
    Type = MethodGenEoS_Thrift::type::CTPM_CPT;
    ModName = "EmpiricalCpIntegration";
}

///< Implementation of the PT correction
long int EmpCpIntegr::PTparam()
{
    double TK, TC, TrK, T_Tst, Ts2, TT, T2, T3, T4, T05,
           Tst2, Tst3, Tst4, Tst05,
           Pb;
    vd     S, G, H, V, Cp;
    int k=-1;
    vector<double> ac;
    ac.resize(16);
    ac = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    TC = AtPT->getCurrentTc();              // get current T in Celsius
    TK = TC + C_to_K;                       // curent T in Kelvin
    Pb = AtPT->getCurrentPb();              // current P in bar
    TrK = AtPrTr->getReferenceTc() + C_to_K;// get reference T in Kelvin

    S = AtPrTr->getRS0();
    G = AtPrTr->getRG0();
    H = AtPrTr->getRH0();
    V = {0.0, 0.0}; // ???
    Cp = {0.0, 0.0};

    // P correction - has to be moved from here!!!
    if(( Substance_class == SubstanceClass::type::GASFLUID /*|| dc[q].pstate[0] == CP_GASI*/ )
            && Pb > 0.0 )
    { // molar volume from the ideal gas law
        V[0] = TK / Pb * R_CONSTANT;
    }

//    aW.twp->devG = dc[q].Gs[1];
//    ??????????????????????????????????????'
//    if( !dc[q].Cp )
//    {
//        aW.twp->Cp = (double)dc[q].Cps[0];
//        goto NEXT;
//    }

    // get Cp interval
    for (unsigned i=0; i<TCinterval.size(); i++)
    {
       if ((TCinterval[i][0] <= TC) && (TCinterval[i][1] > TC))
       {
           k = i;
           break;
       }
    }

    try
    {
        if (k<0)
        throw std::string("Temperature interval for the Cp calculation was not specified! ");
        //exit(1);
    }
    catch (std::string &s)
    {
        cout << s << "Exiting..." << endl;
        return -1;
    }

    T2 = TK * TK;
    T3 = T2 * TK;
    T4 = T3 * TK;
    T05 = std::sqrt( TK );

    for (unsigned i=0; i<CpCoeff[k].size(); i++)
    {
        ac[i] = CpCoeff[k][i];
    }

    Cp[0] = ( ac[0] + ac[1]*TK + ac[2]/T2 + ac[3]/T05 + ac[4]*T2
          + ac[5]*T3 + ac[6]*T4 + ac[7]/T3 + ac[8]/TK + ac[9]*T05 /*+ ac[10]*log(T)*/);

    // Phase transitions
    if (fabs(TK-TrK) > TEMPER_PREC)
    {
        for (unsigned j=0; j<=k; j++)
        {
            if ( j == k )
                TK = AtPT->getCurrentTc() + C_to_K;     // current T is the end T for phase transition Cp calculations
            else TK = TCinterval[j][1] + C_to_K;        // takes the upper bound from the j-th Tinterval
            T2 = TK * TK;
            T3 = T2 * TK;
            T4 = T3 * TK;
            T05 = sqrt( TK );
            if( !j )
                TrK = AtPrTr->getReferenceTc() + C_to_K;            // if j=0 the first interval should contain the reference T (Tcr)
            else  TrK = TCinterval[j][0] + C_to_K;    // if j>0 then we are in a different Tinterval and the reference T becomes the lower bound of the interval
            Tst2 = TrK * TrK;
            Tst3 = Tst2 * TrK;
            Tst4 = Tst3 * TrK;
            Tst05 = sqrt( TrK );
            T_Tst = TK - TrK;
            Ts2 = T_Tst * T_Tst;
            TT = TK / TrK;

            // going trough the phase transitions parameters in FtP
            for (unsigned ft = 0; ft < FtP.size(); ft++)
            if ( j && FtP[ft][0] <= TrK-C_to_K )
            {   // Adding parameters of phase transition
                if ( FtP[ft].size() > 1 )  // dS
                    S[0] += FtP[ft][1];
                if ( FtP[ft].size() > 2 )  // dH
                    H[0] += FtP[ft][2];
                if ( FtP[ft].size() > 3 )  // dV
                    V[0] += FtP[ft][3];
                // More to be added ?
            }

            G[0] -= S[0] * T_Tst;

            for (unsigned i=0; i<CpCoeff[j].size(); i++)
            {
                ac[i] = CpCoeff[j][i];
            }

            S[0] += ( ac[0] * log( TT ) + ac[1] * T_Tst + ac[2] * ( 1./Tst2 - 1./T2 ) / 2.
                 + ac[3] * 2. * ( 1./Tst05 - 1./T05 ) + ac[4] * ( T2 - Tst2 ) / 2.
                 + ac[5] * ( T3 - Tst3 ) / 3. + ac[6] * ( T4 - Tst4 ) / 4.
                 + ac[7] * ( 1./ Tst3 - 1./ T3 ) / 3. + ac[8] * (1. / TrK - 1. / TK )
                 + ac[9] * 2.* ( T05 - Tst05 ) );

            G[0] -= ( ac[0] * ( TK * log( TT ) - T_Tst ) + ac[1] * Ts2 / 2. + ac[2] * Ts2 / TK / Tst2 / 2.
                 + ac[3] * 2. * (T05 - Tst05)*(T05 - Tst05) / Tst05 + ac[4] * ( T3 + 2.*Tst3 - 3.* TK * Tst2 ) / 6.
                 + ac[5] * ( T4 + 3.*Tst4 - 4.*TK * Tst3 ) / 12. + ac[6] * ( T4*TK + 4.*Tst4*TrK - 5.*TK*Tst4 ) / 20.
                 + ac[7] * ( Tst3 - 3.* T2 * TrK + 2.*T3 ) / 6./ T2 / Tst3 + ac[8] * ( TT - 1. - log( TT ))
                 + ac[9] * 2.* ( 2.* TK * T05 - 3.* TK * Tst05 + TrK * Tst05 ) / 3. );

            H[0] += ( ac[0] * T_Tst + ac[1] * ( T2 - Tst2 ) / 2. + ac[2] * ( 1./TrK - 1./TK )
                 + ac[3] * 2. * ( T05 - Tst05 ) + ac[4] * ( T3 - Tst3 ) / 3.
                 + ac[5] * ( T4 - Tst4 ) / 4. + ac[6] * ( T4 * TK - Tst4 * TrK ) / 5
                 + ac[7] * ( 1./ Tst2 - 1./ T2 ) / 2. + ac[8] * log( TT )
                 + ac[9] * 2.* ( TK * T05 - TrK * Tst05 ) / 3. );
        }
    }

    AtPT->setCp0(Cp);
    AtPT->setG0(G);
    AtPT->setH0(H);
    AtPT->setS0(S);
    AtPT->setV0(V);
//    AtPT->setIsProcessed(true);
    return 0;
}
