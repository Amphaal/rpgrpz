#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QSpinBox>
#include <QAction>

#include "src/shared/commands/RPZActions.h"

class MapHelpers : public QToolBar {

    Q_OBJECT

    public:    
        MapHelpers(QWidget * parent = nullptr) : QToolBar(parent) {  

            this->layout()->setMargin(0);
            this->setIconSize(QSize(16, 16));
            this->setMovable(true);
            this->setFloatable(true);

            this->addAction(RPZActions::activateGridIndicator());
            this->addAction(RPZActions::activateScaleIndicator());
            
        }
        
};


