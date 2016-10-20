#include "MinMaxDialog.h"
#include "ui_MinMaxDialog.h"

MinMaxDialog::MinMaxDialog(const char* title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MinMaxDialog)
{
    ui->setupUi(this);
    setWindowTitle(title);
}

MinMaxDialog::~MinMaxDialog()
{
    delete ui;
}


int MinMaxDialog::getMin()
{
  return ui->pmin->value();
}

int MinMaxDialog::getMax()
{
   return ui->pmax->value();
}

int MinMaxDialog::getStep()
{
   return ui->pstep->value();
}

/// Change range of min and max values
void MinMaxDialog::setMinMaxRange(int min, int max, int step)
{
    ui->pmin->setMinimum(min);
    ui->pmin->setMaximum(max);
    ui->pmin->setSingleStep(step);
    ui->pmax->setMinimum(min);
    ui->pmax->setMaximum(max);
    ui->pmax->setSingleStep(step);
}

/// Change range of step value
void MinMaxDialog::setStepRange(int min, int max, int step )
{
    ui->pstep->setMinimum(min);
    ui->pstep->setMaximum(max);
    ui->pstep->setSingleStep(step);
}
