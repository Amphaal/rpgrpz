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
        MapTools(QWidget * parent = nullptr) : QToolBar(parent) {
            
            //self
            this->layout()->setMargin(0);
            this->setIconSize(QSize(16, 16));
            this->setMovable(true);
            this->setFloatable(true);

            QObject::connect(
                this, &QToolBar::actionTriggered,
                this, &MapTools::_onToolSelectionChanged
            );

            this->addAction(RPZActions::usePingTool());
            this->addAction(RPZActions::useMeasurementTool());
            this->addAction(RPZActions::useQuickDrawTool());

        }
 
    signals:
        void toolRequested(const MapTool &tool, bool enabled);

    private:
        void _onToolSelectionChanged(QAction *action) {
            return emit toolRequested(
                (MapTool)action->data().toInt(),
                action->isChecked()
            );
        }

};