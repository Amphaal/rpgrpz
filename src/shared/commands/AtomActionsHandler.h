#pragma once

#include <QWidget>
#include <functional>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/commands/RPZActions.h"

#include "src/shared/hints/AtomsStorage.h"

#include "src/helpers/Clipboard.hpp"

class AtomSelector {
    public:
        virtual const QVector<RPZAtom::Id> selectedIds() const = 0;
};

class AtomActionsHandler {
    
    public:
        AtomActionsHandler(AtomsStorage* master, AtomSelector* selector, QWidget* parent);

        ///
        ///
        ///

        static void undoAlteration(AtomsStorage* master);
        static void redoAlteration(AtomsStorage* master);
        static void copyToClipboard(const QVector<RPZAtom::Id> &ids);
        static void pasteAtomsFromClipboard(AtomsStorage* master);
        static void removeAtoms(AtomsStorage* master, const QVector<RPZAtom::Id> &ids);
        static void moveAtomsToLayer(AtomsStorage* master, const QVector<RPZAtom::Id> &ids, int targetLayer);
        static void alterAtomsVisibility(AtomsStorage* master, const QVector<RPZAtom::Id> &ids, bool hide);
        static void alterAtomsAvailability(AtomsStorage* master, const QVector<RPZAtom::Id> &ids, bool lock);

        ///
        ///
        ///

    private:
        AtomSelector* _selector = nullptr;
        AtomsStorage* _mapMaster = nullptr;

        void _addCopyPasteActionsToShortcuts(QWidget* toAddShortcutsTo);
        void _addUndoRedoActionsToShortcuts(QWidget* toAddShortcutsTo);
        void _addRemoveActionToShortcuts(QWidget* toAddShortcutsTo);

        const QVector<RPZAtom::Id> fromSelector();

};