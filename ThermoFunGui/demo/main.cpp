
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include "ThermoFunMainWindow.h"

int main(int argc, char *argv[])
{

    //jsonio::JsonioSettings::settingsFileName = "Resources/fun-ThermoHubClient-config.json";
    jsonio::JsonioSettings::settingsFileName = "fun-ThermoHubClient-config.json";
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Icons/thermofun-logo-icon.png"));

    TThermoFunMainWin w;
    w.show();
    return a.exec();
}
