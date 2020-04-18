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

#include "src/shared/commands/RPZActions.h"
#include "src/shared/payloads/Payloads.h"

#include <QAction>
#include <QWidget>
#include <QMenu>

#include <QMetaObject>

#include "AtomActionsHandler.h"

class AtomsContextualMenuHandler {
 public:
        AtomsContextualMenuHandler(QWidget* menuParent);

        void invokeMenu(const QList<RPZAtom::Id> &toManipulate, const QPoint &whereToDisplay);

    protected:
        void _moveAtomsToLayer(int layer);
        void _removeAtoms();
        void _undo();
        void _redo();
        void _copy();
        void _paste();
        void _setVisibility(bool hide);
        void _setAvailability(bool lock);

 private:
        QWidget* _menuParent = nullptr;

        QList<RPZAtom::Id> _latestInvokedAtomIds;
        PossibleActionsOnAtomList _latestPossibleActions;

        QAction* _removeAction = nullptr;
        QAction* _copyAction = nullptr;
        QAction* _pasteAction = nullptr;
        QAction* _riseAction = nullptr;
        QAction* _lowerAction = nullptr;
        QAction* _undoAction = nullptr;
        QAction* _redoAction = nullptr;
        QAction* _showAction = nullptr;
        QAction* _hideAction = nullptr;
        QAction* _lockAction = nullptr;
        QAction* _unlockAction = nullptr;

        QAction* _genRemoveAction();
        QList<QAction*> _genLayerActions();
        QList<QAction*> _genUndoRedoActions();
        QList<QAction*> _genCopyPasteActions();
        QList<QAction*> _genVisibilityActions();
        QList<QAction*> _genAvailabilityActions();

};