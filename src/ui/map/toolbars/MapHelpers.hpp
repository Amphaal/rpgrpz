#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QSpinBox>
#include <QAction>

#include "src/shared/commands/RPZActions.h"
#include "src/ui/map/MiniMapView.hpp"


class MapHelpers : public QToolBar {

    Q_OBJECT

    public:    
        MapHelpers(MiniMapView* minimap, QWidget * parent = nullptr) : QToolBar(parent) {  

            this->layout()->setMargin(0);
            this->setIconSize(QSize(16, 16));
            this->setMovable(true);
            this->setFloatable(true);
            
            auto minimapAction = RPZActions::activateMinimap();           
            QObject::connect(
                minimapAction, &QAction::triggered,
                [=](auto checked) {
                    
                    AppContext::settings()->setValue(
                        minimapAction->data().toString(), 
                        checked
                    );

                    minimap->setAsapVisibility(checked);

                }
            );

            this->addAction(RPZActions::activateGridIndicator());
            this->addAction(RPZActions::activateScaleIndicator());
            this->addAction(minimapAction);

        }
        
};


