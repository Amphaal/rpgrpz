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

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
#include <QGraphicsItem>
#include <QMenu>
#include <QHeaderView>
#include <QKeyEvent>
#include <QTimer>

#include "src/shared/commands/RPZActions.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

#include "src/ui/mapLayout/_base/LockAndVisibilityDelegate.h"

#include "src/ui/mapLayout/model/MapLayoutModel.hpp"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class MapLayoutTree : public QTreeView, public AtomSelector {

    Q_OBJECT

    public:
        MapLayoutTree(QWidget* parent = nullptr);

        const QList<RPZAtom::Id> selectedIds() const override;
        MapLayoutModel* mlModel = nullptr;

    protected:
        void contextMenuEvent(QContextMenuEvent *event) override;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload);

    private:
        bool _preventSelectionNotification = false;
        QTimer _selectionDebouncer;

        AtomsContextualMenuHandler* _menuHandler = nullptr;
        AtomActionsHandler* _atomActionsHandler = nullptr;
        
        void _handleHintsSignalsAndSlots();

};
