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
#include <functional>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/commands/RPZActions.h"

#include "src/shared/hints/HintThread.hpp"

#include "src/helpers/Clipboard.hpp"

#include "src/helpers/Authorisations.hpp"

class AtomSelector {
 public:
    virtual const QList<RPZAtom::Id> selectedIds() const = 0;
};

class AtomActionsHandler {
 public:
    AtomActionsHandler(AtomSelector* selector, QWidget* parent);

    ///
    ///
    ///

    static void undoAlteration();
    static void redoAlteration();
    static void copyToClipboard(const QList<RPZAtom::Id> &ids);
    static void pasteAtomsFromClipboard();
    static void removeAtoms(const QList<RPZAtom::Id> &ids);
    static void moveAtomsToLayer(const QList<RPZAtom::Id> &ids, int targetLayer);
    static void alterAtomsVisibility(const QList<RPZAtom::Id> &ids, bool hide);
    static void alterAtomsAvailability(const QList<RPZAtom::Id> &ids, bool lock);

    ///
    ///
    ///

 private:
    AtomSelector* _selector = nullptr;

    void _addCopyPasteActionsToShortcuts(QWidget* toAddShortcutsTo);
    void _addUndoRedoActionsToShortcuts(QWidget* toAddShortcutsTo);
    void _addRemoveActionToShortcuts(QWidget* toAddShortcutsTo);

    const QList<RPZAtom::Id> fromSelector();
};
