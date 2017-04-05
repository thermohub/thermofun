#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <string>
#include <QMainWindow>
#include <QSettings>
using namespace std;

#include "bsonio/thrift_schema.h"
#include "bsonui/BSONUIBase.h"

namespace Ui {
class TestMainWindow;
}

class TestMainWindow : public QMainWindow
{
    Q_OBJECT

    // Internal settings
    QSettings *mainSettings;        ///< Properties for program

    QString SchemDir;               ///< Path to schemas directory
    //QString LocalDBDir;             ///< Path to Local EJDB
    //bool useLocalDB;                ///< Flag for using local DB (no server )

    // Internal data
    bsonio::ThriftSchema schema;
    list<bsonui::BSONUIBase*> bsonuiWindows;
    bsonui::onEventfunction onCloseEvent;
    bsonui::ShowWidgetFunction showWidget;

    // Work functions

    void setDefValues();
    void getDataFromPreferences();
    void setActions();
    /// Read all thrift schemas from Dir
    void readSchemaDir( const QString& dirPath );
    /// Open new BSONUIWidget window
    void newSchemaWin(const char* testschema );

    void closeEvent(QCloseEvent* e)
    {
      closeAll();
      QWidget::closeEvent(e);
    }

public slots:

    // update after change preferences
    void updateViewMenu();
    void updateModel();
    void updateTable();
    void updateDB();
    /// Close All BSONUIWidget windows
    void closeAll();

    // File
    void CmNewTCorPT();

    // View
    void CmShowComments();
    void CmDisplayEnums();
    void CmEditID();
    void CmExpanded();

    // Help
    void CmHelpAbout(){}
    void CmHelpAuthors(){}
    void CmHelpLicense(){}
    void CmHelpContens(){}
    void CmSettingth();

public:
    explicit TestMainWindow(QWidget *parent = 0);
    ~TestMainWindow();

private:

    Ui::TestMainWindow *ui;
};

#endif // TMAINWINDOW_H
