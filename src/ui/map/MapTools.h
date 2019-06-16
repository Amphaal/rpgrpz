#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QSpinBox>
#include <QAction>

#include "src/shared/commands/RPZActions.h"

class MapTools : public QToolBar {

    Q_OBJECT

    public:    
        enum Actions { 
            ResetView,
            ResetTool
        };

        MapTools(QWidget * parent = nullptr); 
    
    signals:
        void actionRequested(const MapTools::Actions &action);

    private:
        void _onToolSelectionChanged(QAction *action);
};