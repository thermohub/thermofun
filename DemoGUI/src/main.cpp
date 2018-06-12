//#include "BSONUITestMainWindow.h"

#include "jsonio/io_settings.h"
#include "jsonui/JSONUIBase.h"
#include "ThermoFunWidgetNew.h"
//#include "jsonio/nservice.h"
#include "jsonui/preferences.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

//using namespace jsonui;
//using namespace jsonio;


/*namespace jsonio {

std::string JsonioSettings::settingsFileName = "jsonui.json";
}*/

int main(int argc, char *argv[])
{

    jsonio::JsonioSettings::settingsFileName = "Resources/ThermoFunDemoGUI.json";
    QApplication a(argc, argv);

    std::list<jsonui::JSONUIBase*> bsonuiWindows;

    jsonui::onEventfunction onCloseEvent;
    jsonui::ShowWidgetFunction showWidget;
//    TBSONUITestWin w;
//    w.show();


    try{
          jsonui::JSONUIBase* testWidget;
          testWidget = new ThermoFunWidgetNew( /*this*/ );

          onCloseEvent = [](QMainWindow*)
          {
              return true;
          };

          testWidget->setOnCloseEventFunction(onCloseEvent);
          testWidget->setShowWidgetFunction(showWidget);
          bsonuiWindows.push_back(testWidget);
          testWidget->show();

      }
     catch(jsonio::jsonio_exeption& e)
    {}
//     {
//         QMessageBox::critical( a, e.title(), e.what() );
//     }
//     catch(std::exception& e)
//      {
//         QMessageBox::critical( a, "std::exception", e.what() );
//      }
    return a.exec();
}
