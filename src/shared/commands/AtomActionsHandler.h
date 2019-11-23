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