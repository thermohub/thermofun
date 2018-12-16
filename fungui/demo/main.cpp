
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include "ThermoFunMainWindow.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
    jsonio::JsonioSettings::settingsFileName = "Resources/fun-dbclient-config-win.json";
#else
    jsonio::JsonioSettings::settingsFileName = "Resources/fun-dbclient-config.json";
#endif
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Icons/thermofun-logo-icon.png"));

    TThermoFunMainWin w;
    w.show();
    return a.exec();
}
