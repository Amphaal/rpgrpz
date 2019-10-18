#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QSpinBox>
#include <QAction>

#include "src/shared/commands/RPZActions.h"

class MapActions : public QToolBar {

    Q_OBJECT

    public:    
        MapActions(QWidget * parent = nullptr) : QToolBar(parent) {
            
            //self
            this->layout()->setMargin(0);
            this->setIconSize(QSize(16, 16));
            this->setMovable(true);
            this->setFloatable(true);

            QObject::connect(
                this, &QToolBar::actionTriggered,
                this, &MapActions::_onActionSelectionChanged
            );

            this->addSeparator();

            //reset tool
            auto resetTool = RPZActions::resetTool();
            this->addAction(resetTool);

            //reset view
            auto resetV = RPZActions::resetView();
            this->addAction(resetV);

        }
 
    signals:
        void actionRequested(const MapAction &action);

    private:
        void MapActions::_onActionSelectionChanged(QAction *action) {
            return emit actionRequested((MapAction)action->data().toInt());
        }

};