#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <QMainWindow>
#include "jsonui/JSONUIBase.h"

namespace Ui {
class TThermoFunMainWindow;
}

class TThermoFunMainWin : public QMainWindow
{
    Q_OBJECT

    // Internal data
    std::list<jsonui::JSONUIBase*> bsonuiWindows;
    jsonui::onEventfunction onCloseEvent;
    jsonui::ShowWidgetFunction showWidget;

    // Work functions
    void setActions();

    /// Open new JSONUIWidget window
    void newSchemaWin(const char* testschema );

    void closeEvent(QCloseEvent* e);


public slots:

    // update after change preferences
    void updateViewMenu();
    void updateModel();
    void updateTable();
    void updateDB();
    /// Close All JSONUIWidget windows
    void closeAll();

    // File
    void CmNewElement()
    { newSchemaWin( "VertexElement" ); }
    void CmNewSubstance()
    { newSchemaWin( "VertexSubstance" ); }
    void CmNewReaction()
    { newSchemaWin( "VertexReaction" ); }
    void CmNewReactionSet()
    { newSchemaWin( "VertexReactionSet" ); }
    void CmNewDataSource()
    { newSchemaWin( "VertexDataSource" ); }
    void CmNewThermoDataSet()
    { newSchemaWin( "VertexThermoDataSet" ); }
    void CmNewEdge();

    // Tools
    void CmThermoFun();

    // Help
    void CmHelpAbout();
    void CmHelpAuthors();
    void CmHelpLicense();
    void CmHelpContens();

    /// Load all elements properties
    void setAllElements();

    void onDeleteVertex();
    void onDeleteEdge();
    void onLoadGraph();

public:
    explicit TThermoFunMainWin(QWidget *parent = 0);
    ~TThermoFunMainWin();

    /// Open new EdgesWidget  or JSONUIWidget  window
    void OpenNewWidget( bool isVertex, const std::string& testschema, const std::string& recordkey,
                        const jsonio::DBQueryData& query = jsonio::emptyQuery );

private:

    Ui::TThermoFunMainWindow *ui;
};

#endif // TMAINWINDOW_H
