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