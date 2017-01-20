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

    int calcSize( int mode, int NT, int NP);
    void initTPvectors( std::vector<double>& Tvec, std::vector<double>& Pvec);

public:
    explicit TPSetDialog(QWidget *parent = 0);
    ~TPSetDialog();

    QString getTUnits();
    QString getPUnits();
    std::vector< std::vector<double> > getTP();

private:

    Ui::TPSetDialog *ui;
};

#endif // TPSETDIALOG_H
