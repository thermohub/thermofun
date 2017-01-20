#include "algorithm"
#include "TPSetDialog.h"
#include "ui_TPSetDialog.h"

TPSetDialog::TPSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TPSetDialog)
{
    ui->setupUi(this);
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


std::vector<std::vector<double>> TPSetDialog::getTP()
{
   std::vector<std::vector<double>> tppairs;

   return tppairs;
}


int TPSetDialog::calcSize( int mode, int NT, int NP)
{
    int NV;

    switch( mode )
    {
       default:
       case 0: // the user must enter all P and T values in xP and xT vectors
           NV = std::max( NP, NT );
           break;
      case 1: // increments in cycle on P nested into cycle on T
      case 2: // increments in cycle on T nested into cycle on P (default)
           NV = NP * NT;
           break;
      case 3: // parallel increments of T and P in one cycle
           NV = std::max( NP, NT );
           break;
    }
    return NV;
}

void TPSetDialog::initTPvectors( std::vector<double>& Tvec, std::vector<double>& Pvec)
{
    int j, i, mode=0, NT=1, NP=1;
    int NV = calcSize( mode, NT, NP );

    Tvec.clear();
    Pvec.clear();

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
         {
            Tvec.push_back(0.);
            Pvec.push_back(0.);
         }
        break;
     case 1: // increments in cycle on P nested into cycle on T
        for( i=0; i<NT; i++ )
        {
          Tij = Ti0 + Ti2 * i;
          for( j=0; j<NP; j++ )
          {
            Pij = Pi0 + Pi2 * j;
            Tvec.push_back(Tij);
            Pvec.push_back(Pij);
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
            Tvec.push_back(Tij);
            Pvec.push_back(Pij);
        }
      }
      break;
    case 3: // parallel increments of T and P in one cycle
      for( i=0; i<NV; i++ )
      {
          Pij = Pi0 + Pi2 * i;
          Tij = Ti0 + Ti2 * i;
          Tvec.push_back(Tij);
          Pvec.push_back(Pij);
      }
    break;
   }
}
