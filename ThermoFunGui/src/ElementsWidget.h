#ifndef ELEMENTSWIDGET_H
#define ELEMENTSWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#ifdef FROM_SRC
#include "../../ThermoFun/Common/formuladata.h"
#endif
#ifndef FROM_SRC
#include "ThermoFun/Common/formuladata.h"
#endif

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
    explicit ElementsWidget(QWidget *parent = nullptr);
    ~ElementsWidget();

    void  setElementList( const std::vector<ThermoFun::ElementKey>& elements );
    void  selectElementList( const std::vector<ThermoFun::ElementKey>& elements );
    void allSelected( std::vector<ThermoFun::ElementKey>& elementKeys ) const;

private:
    Ui::ElementsWidget *ui;

    std::vector<int> aBtmId1;         // lists correspondanse betvin bgElem
    std::vector<ThermoFun::ElementKey> aICkey1;  // buttons and IComp records
    std::vector<int> aBtmId2;         // lists correspondanse betvin bgOther
    std::vector<ThermoFun::ElementKey> aICkey2;  // buttons and IComp records
    QButtonGroup* bgElem;
    QButtonGroup* bgOther;

};

#endif // ELEMENTSWIDGET_H
