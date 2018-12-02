#include "ElementsWidget.h"
#include "ui_ElementsWidget.h"

ElementsWidget::ElementsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ElementsWidget)
{
    ui->setupUi(this);
    setUpButtonGroups();

    connect( ui->pbE_REE, SIGNAL( clicked() ), this, SLOT( CmResetREE() ) );
    connect( ui->pbE_ACT, SIGNAL( clicked() ), this, SLOT( CmResetACT() ) );
    connect( ui->cbIsotopes, SIGNAL( clicked() ),this, SLOT( SetIsotopes() ) );
}

ElementsWidget::~ElementsWidget()
{
    delete ui;
}

void ElementsWidget::SetIsotopes()
{
}

void ElementsWidget::CmSelectAll()
{
    int ii;
    for( ii=0; ii<bgOther->buttons().count(); ii++ )
      if( bgOther->button(ii)->isEnabled() )
        bgOther->button(ii)->setChecked ( true );
    for( ii=0; ii<99/*bgElem->buttons()->count()-1*/; ii++ )
        if( bgElem->button(ii)->isEnabled() )
         bgElem->button(ii)->setChecked ( true );
}

void ElementsWidget::CmResetREE()
{
    for(int ii=57; ii<=71; ii++ )
        if( bgElem->button(ii)->isEnabled() )
         bgElem->button(ii)->setChecked ( true );
}

void ElementsWidget::CmResetACT()
{
    for(int ii=89; ii<=98; ii++ )
        if( bgElem->button(ii)->isEnabled() )
         bgElem->button(ii)->setChecked ( true );
}

void ElementsWidget::allSelected( vector<ThermoFun::ElementKey>& elementKeys ) const
{
    unsigned int ii;
    elementKeys.clear();
    for( ii=0; ii<aBtmId1.size(); ii++ )
    {
     if( bgElem->button( aBtmId1[ii] )->isChecked()  )
        elementKeys.push_back( aICkey1[ii] );
    }

    for( ii=0; ii<aBtmId2.size(); ii++ )
    {
     if( bgOther->button( aBtmId2[ii] )->isChecked()  )
        elementKeys.push_back( aICkey2[ii] );
    }
}

void ElementsWidget::setElementList(const vector<ThermoFun::ElementKey> &elements )
{
    ResetData();
    EmptyData();

    QAbstractButton* bb;
    int nmbOther=0;
    aBtmId1.clear();
    aICkey1.clear();
    aBtmId2.clear();
    aICkey2.clear();

   //const DBElementsData& allElements = ChemicalFormula::getDBElements();

   for(auto const &elkey : elements)
   {
      auto  eldata = ThermoFun::ChemicalFormula::getDBElements().find(elkey);
      if( eldata ==  ThermoFun::ChemicalFormula::getDBElements().end() )
         continue;

      string  name =elkey.symbol;
      int aIndMT = eldata->second.number;
      if( aIndMT == -1 || aIndMT > 101 ) // additional
      {
         jsonio::jsonioErrIf( nmbOther>12, name, "More than 11 additional Elements!");
         bb = bgOther->button(nmbOther);
         bb->setText( tr( name.c_str() ) );
         bb->setEnabled( true );
         aBtmId2.push_back( nmbOther );
         aICkey2.push_back( elkey );
         nmbOther ++;
     }
     else //elements
     {
       bb = bgElem->button( aIndMT );
       jsonio::jsonioErrIf( !bb, name, "E01EDrem: Invalid Element ");
       aBtmId1.push_back( aIndMT );
       aICkey1.push_back( elkey );
       bb->setEnabled( true );
     }
  }
}

void ElementsWidget::selectElementList( const vector<ThermoFun::ElementKey>& elements )
{
   for(auto const &elkey : elements)
   {
      auto  eldata = ThermoFun::ChemicalFormula::getDBElements().find(elkey);
      if( eldata ==  ThermoFun::ChemicalFormula::getDBElements().end() )
         continue;

      string  name =elkey.symbol;
      int aIndMT = eldata->second.number;
      if( aIndMT == -1 || aIndMT > 101 ) // additional
      {
          for(int  ii=0; ii<bgOther->buttons().count(); ii++ )
            if( name == bgOther->button(ii)->text().toStdString() )
              bgOther->button(ii)->setChecked ( true );
      }
     else //elements
     {
       QAbstractButton* bb = bgElem->button( aIndMT );
       if( bb->isEnabled() )
         bb->setChecked( true);
     }
  }
}


void 	ElementsWidget::setUpButtonGroups()
{
    bgElem = new QButtonGroup( ui->bgElemBox);
    bgElem->setExclusive(false);
    bgElem->addButton(ui->pbE_0, 0);
    bgElem->addButton(ui->pbE_1, 1);
    bgElem->addButton(ui->pbE_2, 2);
    bgElem->addButton(ui->pbE_3, 3);
    bgElem->addButton(ui->pbE_4, 4);
    bgElem->addButton(ui->pbE_5, 5);
    bgElem->addButton(ui->pbE_6, 6);
    bgElem->addButton(ui->pbE_7, 7);
    bgElem->addButton(ui->pbE_8, 8);
    bgElem->addButton(ui->pbE_9, 9);
    bgElem->addButton(ui->pbE_10, 10);
    bgElem->addButton(ui->pbE_11, 11);
    bgElem->addButton(ui->pbE_12, 12);
    bgElem->addButton(ui->pbE_13, 13);
    bgElem->addButton(ui->pbE_14, 14);
    bgElem->addButton(ui->pbE_15, 15);
    bgElem->addButton(ui->pbE_16, 16);
    bgElem->addButton(ui->pbE_17, 17);
    bgElem->addButton(ui->pbE_18, 18);
    bgElem->addButton(ui->pbE_19, 19);
    bgElem->addButton(ui->pbE_20, 20);
    bgElem->addButton(ui->pbE_21, 21);
    bgElem->addButton(ui->pbE_22, 22);
    bgElem->addButton(ui->pbE_23, 23);
    bgElem->addButton(ui->pbE_24, 24);
    bgElem->addButton(ui->pbE_25, 25);
    bgElem->addButton(ui->pbE_26, 26);
    bgElem->addButton(ui->pbE_27, 27);
    bgElem->addButton(ui->pbE_28, 28);
    bgElem->addButton(ui->pbE_29, 29);
    bgElem->addButton(ui->pbE_30, 30);
    bgElem->addButton(ui->pbE_31, 31);
    bgElem->addButton(ui->pbE_32, 32);
    bgElem->addButton(ui->pbE_33, 33);
    bgElem->addButton(ui->pbE_34, 34);
    bgElem->addButton(ui->pbE_35, 35);
    bgElem->addButton(ui->pbE_36, 36);
    bgElem->addButton(ui->pbE_37, 37);
    bgElem->addButton(ui->pbE_38, 38);
    bgElem->addButton(ui->pbE_39, 39);
    bgElem->addButton(ui->pbE_40, 40);
    bgElem->addButton(ui->pbE_41, 41);
    bgElem->addButton(ui->pbE_42, 42);
    bgElem->addButton(ui->pbE_43, 43);
    bgElem->addButton(ui->pbE_44, 44);
    bgElem->addButton(ui->pbE_45, 45);
    bgElem->addButton(ui->pbE_46, 46);
    bgElem->addButton(ui->pbE_47, 47);
    bgElem->addButton(ui->pbE_48, 48);
    bgElem->addButton(ui->pbE_49, 49);
    bgElem->addButton(ui->pbE_50, 50);
    bgElem->addButton(ui->pbE_51, 51);
    bgElem->addButton(ui->pbE_52, 52);
    bgElem->addButton(ui->pbE_53, 53);
    bgElem->addButton(ui->pbE_54, 54);
    bgElem->addButton(ui->pbE_55, 55);
    bgElem->addButton(ui->pbE_56, 56);
    bgElem->addButton(ui->pbE_57, 57);
    bgElem->addButton(ui->pbE_58, 58);
    bgElem->addButton(ui->pbE_59, 59);
    bgElem->addButton(ui->pbE_60, 60);
    bgElem->addButton(ui->pbE_61, 61);
    bgElem->addButton(ui->pbE_62, 62);
    bgElem->addButton(ui->pbE_63, 63);
    bgElem->addButton(ui->pbE_64, 64);
    bgElem->addButton(ui->pbE_65, 65);
    bgElem->addButton(ui->pbE_66, 66);
    bgElem->addButton(ui->pbE_67, 67);
    bgElem->addButton(ui->pbE_68, 68);
    bgElem->addButton(ui->pbE_69, 69);
    bgElem->addButton(ui->pbE_70, 70);
    bgElem->addButton(ui->pbE_71, 71);
    bgElem->addButton(ui->pbE_72, 72);
    bgElem->addButton(ui->pbE_73, 73);
    bgElem->addButton(ui->pbE_74, 74);
    bgElem->addButton(ui->pbE_75, 75);
    bgElem->addButton(ui->pbE_76, 76);
    bgElem->addButton(ui->pbE_77, 77);
    bgElem->addButton(ui->pbE_78, 78);
    bgElem->addButton(ui->pbE_79, 79);
    bgElem->addButton(ui->pbE_80, 80);
    bgElem->addButton(ui->pbE_81, 81);
    bgElem->addButton(ui->pbE_82, 82);
    bgElem->addButton(ui->pbE_83, 83);
    bgElem->addButton(ui->pbE_84, 84);
    bgElem->addButton(ui->pbE_85, 85);
    bgElem->addButton(ui->pbE_86, 86);
    bgElem->addButton(ui->pbE_87, 87);
    bgElem->addButton(ui->pbE_88, 88);
    bgElem->addButton(ui->pbE_89, 89);
    bgElem->addButton(ui->pbE_90, 90);
    bgElem->addButton(ui->pbE_91, 91);
    bgElem->addButton(ui->pbE_92, 92);
    bgElem->addButton(ui->pbE_93, 93);
    bgElem->addButton(ui->pbE_94, 94);
    bgElem->addButton(ui->pbE_95, 95);
    bgElem->addButton(ui->pbE_96, 96);
    bgElem->addButton(ui->pbE_97, 97);
    bgElem->addButton(ui->pbE_98, 98);

//    bgElem->addButton(ui->pbE_100, 100);
//    bgElem->addButton(ui->pbE_101, 101);

    bgOther = new QButtonGroup( ui->bgOtherBox);
    bgOther->setExclusive(false);
     bgOther->addButton(ui->pbA_0, 0);
     bgOther->addButton(ui->pbA_1, 1);
     bgOther->addButton(ui->pbA_2, 2);
     bgOther->addButton(ui->pbA_3, 3);
     bgOther->addButton(ui->pbA_4, 4);
     bgOther->addButton(ui->pbA_5, 5);
     bgOther->addButton(ui->pbA_6, 6);
     bgOther->addButton(ui->pbA_7, 7);
     bgOther->addButton(ui->pbA_8, 8);
     bgOther->addButton(ui->pbA_9, 9);
     bgOther->addButton(ui->pbA_10, 10);
     bgOther->addButton(ui->pbA_11, 11);
}

void ElementsWidget::EmptyData()
{
 int ii;
 for( ii=0; ii<bgOther->buttons().count(); ii++ )
 {     bgOther->button(ii)->setEnabled( false );
       bgOther->button(ii)->setText( tr( "...") );
 }
 for( ii=0; ii<99/*bgElem->buttons()->count()-1*/; ii++ ) // we have not 99
      bgElem->button(ii)->setEnabled( false );
}

void ElementsWidget::ResetData()
{
 int ii;
 for( ii=0; ii<bgOther->buttons().count(); ii++ )
   bgOther->button(ii)->setChecked ( false );
 for( ii=1; ii<99/*bgElem->buttons()->count()-1*/; ii++ ) // 0 always
      bgElem->button(ii)->setChecked ( false );
}

