#ifndef ELEMENTSWIDGET_H
#define ELEMENTSWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include "thermofun/DBClient/formuladata.h"

namespace Ui {
class ElementsWidget;
}

class ElementsWidget : public QWidget
{
    Q_OBJECT

    void  setUpButtonGroups();
    void  EmptyData();
    void  ResetData();

public slots:
    void CmSelectAll();
    void CmReset()
    {
      ResetData();
    }
    void CmResetREE();
    void CmResetACT();
    void SetIsotopes();

public:
    explicit ElementsWidget(QWidget *parent = 0);
    ~ElementsWidget();

    void  setElementList( const vector<ThermoFun::ElementKey>& elements );
    void  selectElementList( const vector<ThermoFun::ElementKey>& elements );
    void allSelected( vector<ThermoFun::ElementKey>& elementKeys ) const;

private:
    Ui::ElementsWidget *ui;

    vector<int> aBtmId1;         // lists correspondanse betvin bgElem
    vector<ThermoFun::ElementKey> aICkey1;  // buttons and IComp records
    vector<int> aBtmId2;         // lists correspondanse betvin bgOther
    vector<ThermoFun::ElementKey> aICkey2;  // buttons and IComp records
    QButtonGroup* bgElem;
    QButtonGroup* bgOther;

};

#endif // ELEMENTSWIDGET_H
