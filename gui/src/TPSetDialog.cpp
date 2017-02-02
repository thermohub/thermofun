#include "algorithm"
#include "TPSetDialog.h"
#include "ui_TPSetDialog.h"

TPSetDialog::TPSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TPSetDialog)
{
    ui->setupUi(this);
    PairwiseChecked();

    QObject::connect( ui->pTfrom, SIGNAL(valueChanged( double  )), this, SLOT(TChange()));
    QObject::connect( ui->pTuntil, SIGNAL(valueChanged( double  )), this, SLOT(TChange()));
    QObject::connect( ui->pTstep, SIGNAL(valueChanged( double  )), this, SLOT(TChange()));
    QObject::connect( ui->pPfrom, SIGNAL(valueChanged( double  )), this, SLOT(PChange()));
    QObject::connect( ui->pPuntil, SIGNAL(valueChanged( double  )), this, SLOT(PChange()));
    QObject::connect( ui->pPstep, SIGNAL(valueChanged( double  )), this, SLOT(PChange()));
    QObject::connect( ui->pMode, SIGNAL(currentIndexChanged( int ) ), this, SLOT(PairwiseChecked()));
}

TPSetDialog::~TPSetDialog()
{
    delete ui;
}

QString TPSetDialog::getTUnits()
{
  return ui->pTun->currentText();
}

QString TPSetDialog::getPUnits()
{
  return ui->pPun->currentText();
}

std::vector<std::vector<double>> TPSetDialog::getTPpairs()
{
   std::vector<std::vector<double>> tppairs;

   long int j, i;
   long int mode = ui->pMode->currentIndex();
   long int NT = ui->pNT->value();
   long int NP = ui->pNP->value();

   long int NV = calcSize( mode, NT, NP );
   double Ti0 = ui->pTfrom->value();
   double Ti2 = ui->pTstep->value();
   double Pi0 = ui->pPfrom->value();
   double Pi2 = ui->pPstep->value();
   double Tij, Pij;

   // insert values to arrays  rp->T, rp->P
   switch( mode )
   {
     default:
     case 0: // the user must enter all P and T values in xP and xT vectors
        for( i=0; i<NV; i++ )
           tppairs.push_back({0., 0.});
       break;
    case 1: // increments in cycle on P nested into cycle on T
       for( i=0; i<NT; i++ )
       {
         Tij = Ti0 + Ti2 * i;
         for( j=0; j<NP; j++ )
         {
           Pij = Pi0 + Pi2 * j;
           tppairs.push_back({Tij, Pij});
         }
      }
      break;
   case 2: // increments in cycle on T nested into cycle on P (default)
      for( i=0; i<NP; i++ )
      {
        Pij = Pi0 + Pi2 * i;
        for( j=0; j<NT; j++ )
        {
           Tij = Ti0 + Ti2 * j;
           tppairs.push_back({Tij, Pij});
       }
     }
     break;
   case 3: // parallel increments of T and P in one cycle
     for( i=0; i<NV; i++ )
     {
         Pij = Pi0 + Pi2 * i;
         Tij = Ti0 + Ti2 * i;
         tppairs.push_back({Tij, Pij});
     }
   break;
  }
   return tppairs;
}


void TPSetDialog::TChange()
{
    double Tai[4];
    long int nT;
    Tai[0] = ui->pTfrom->value();
    Tai[1] = ui->pTuntil->value();
    Tai[2] = ui->pTstep->value();
    nT = calcNpoints( Tai );
    ui->pNT->setValue(nT);
}

void TPSetDialog::PChange()
{
  double Pai[4];
  long int nP;
  Pai[0] = ui->pPfrom->value();
  Pai[1] = ui->pPuntil->value();
  Pai[2] = ui->pPstep->value();
  nP = calcNpoints( Pai );
  ui->pNP->setValue(nP);
}

void TPSetDialog::PairwiseChecked()
{
  bool modeNoPair = ( ui->pMode->currentIndex() != 0);

  ui->pTfrom->setEnabled ( modeNoPair );
  ui->pTuntil->setEnabled ( modeNoPair );
  ui->pTstep->setEnabled ( modeNoPair );
  ui->pPfrom->setEnabled ( modeNoPair );
  ui->pPuntil->setEnabled ( modeNoPair );
  ui->pPstep->setEnabled ( modeNoPair );

  ui->pNP->setEnabled ( !modeNoPair );
  ui->pNT->setEnabled ( !modeNoPair );
}


long int TPSetDialog::calcSize( long int mode, long int NT, long int NP)
{
    long int NV;
    switch( mode )
    {
       default:
       case 3: // parallel increments of T and P in one cycle
       case 0: // the user must enter all P and T values in xP and xT vectors
           NV = std::max( NP, NT );
           break;
       case 1: // increments in cycle on P nested into cycle on T
       case 2: // increments in cycle on T nested into cycle on P (default)
           NV = NP * NT;
           break;
    }
    return NV;
}

/// Calculate number of points from iterators
long int  TPSetDialog::calcNpoints( double Tai[4] )
{
  long int nPoints = 0;

  if( fabs(Tai[2]) <= 1e-30 )
          nPoints = 1;
  else
          nPoints = (long int)( (Tai[1]-Tai[0]) / Tai[2] )+1;

  if( nPoints < 1 )
     nPoints = 1;

   return nPoints;
}
