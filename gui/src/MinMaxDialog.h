#ifndef MINMAXDIALOG_H
#define MINMAXDIALOG_H

#include <QDialog>

namespace Ui {
class MinMaxDialog;
}

class MinMaxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MinMaxDialog(const char* title, QWidget *parent = 0);
    ~MinMaxDialog();

    int getMin();
    int getMax();
    int getStep();

    /// Change range of min and max values
    void setMinMaxRange(int min, int max, int step = 1);
    /// Change range of step value
    void setStepRange(int min, int max, int step = 1);

private:
    Ui::MinMaxDialog *ui;
};

#endif // MINMAXDIALOG_H
