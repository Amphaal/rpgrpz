#pragma once

#include <QString>
#include <QAction>
#include <QIcon>
#include "src/helpers/_appContext.h"

class RPZActions {
    public:
        static QAction* redo();
        static QAction* undo();
        static QAction* copy();
        static QAction* paste();
        static QAction* createFolder();
        static QAction* remove();
        static QAction* raiseAtom(int targetLayer);
        static QAction* lowerAtom(int targetLayer);
        static QAction* showAtom();
        static QAction* hideAtom();
        static QAction* resetView();
        static QAction* resetTool();
        static QAction* lockAtom();
        static QAction* unlockAtom();
        static QAction* quit();
        static QAction* patchnote();
        static QAction* checkUpdates();
        static QAction* openInternalDataFolder();
        static QAction* openLatestLog();
        static QAction* openFullLog();
        static QAction* openMaintenanceTool();
        static QAction* createANewMap();
        static QAction* loadAMap();
        static QAction* saveTheMap();
        static QAction* saveTheMapAs();
};