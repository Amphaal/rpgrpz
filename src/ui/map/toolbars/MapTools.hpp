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
// different license and copyright still refer to this GPL.

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

            _inst = this;

        }

        static MapTools* get() {
            return _inst;
        }

        void onToolChange(const MapTool &tool) {
            auto associatedAction = _findActionFromTool(tool);
            this->_uncheckActions(associatedAction);
        }

 signals:
        void toolRequested(const MapTool &tool, bool enabled);

 private:
        static inline MapTools* _inst = nullptr;

        QAction* _findActionFromTool(const MapTool &tool) {
            for(auto action : this->actions()) {
                auto actionTool = this->_mapTool(action);
                if(tool == actionTool) return action;
            }
            return nullptr;
        }

        MapTool _mapTool(QAction *action) {
            return (MapTool)action->data().toInt();
        }

        void _onToolSelectionChanged(QAction *action) {
            
            this->_uncheckActions(action);

            auto associatedTool = this->_mapTool(action);

            return emit toolRequested(associatedTool, action->isChecked());

        }

        void _uncheckActions(QAction * toMaintainState = nullptr) {
            
            auto allActions = this->actions();
            
            //remove from list
            if(toMaintainState) {
                auto found = allActions.indexOf(toMaintainState);
                if(found > -1) allActions.removeAt(found);
            }
            
            //uncheck
            this->blockSignals(true);
            for(auto action : allActions) {
                action->setChecked(false);
            }
            this->blockSignals(false);

        }

};