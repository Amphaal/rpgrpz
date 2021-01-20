// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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
#include "src/ui/map/MiniMapView.hpp"

class MapHelpers : public QToolBar {
    Q_OBJECT

 public:
    explicit MapHelpers(MiniMapView* minimap, QWidget * parent = nullptr) : QToolBar(parent) {
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
        });

        this->addAction(RPZActions::activateGridIndicator());
        this->addAction(RPZActions::activateScaleIndicator());
        this->addAction(minimapAction);
    }
};
