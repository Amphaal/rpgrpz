// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#include <QString>
#include <QAction>
#include <QIcon>
#include "src/helpers/_appContext.h"

enum class MapAction { 
    ResetView,
    ResetTool
};

enum class MapTool {
    Default, 
    Atom, 
    Scroll,
    Ping,
    Measure,
    QuickDraw,
    Walking
};

class RPZActions {
    public:
        static QAction* sentry();
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
        static QAction* activateGridIndicator();
        static QAction* activateScaleIndicator();
        static QAction* activateMinimap();
        static QAction* usePingTool();
        static QAction* useMeasurementTool();
        static QAction* useQuickDrawTool();
};