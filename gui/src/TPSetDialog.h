#ifndef TPSETDIALOG_H
#define TPSETDIALOG_H

#include <vector>
#include <QDialog>
#include <QString>

namespace Ui {
class TPSetDialog;
}

class TPSetDialog : public QDialog
{
    Q_OBJECT

    long int calcSize( long int mode, long int NT, long int NP);
    /// Calculate number of points from iterators
    long int calcNpoints( double Tai[4] );

protected slots:

    void PChange();
    void TChange();
    void PairwiseChecked();

public:
    explicit TPSetDialog(QWidget *parent = 0);
    ~TPSetDialog();

    QString getTUnits();
    QString getPUnits();
    std::vector<std::vector<double>> getTPpairs();

private:

    Ui::TPSetDialog *ui;
};

#endif // TPSETDIALOG_H
