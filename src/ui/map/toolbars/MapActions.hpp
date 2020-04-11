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

// Any graphical or audio resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

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
        void _onActionSelectionChanged(QAction *action) {
            return emit actionRequested((MapAction)action->data().toInt());
        }

};