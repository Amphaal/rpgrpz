#pragma once

#include "src/shared/hints/AtomsStorage.h"
#include "src/shared/commands/RPZActions.h"
#include "src/shared/payloads/Payloads.h"

#include "src/ui/_others/ClientBindable.h"

#include <QAction>
#include <QWidget>
#include <QMenu>

#include <QMetaObject>

class AtomsContextualMenuHandler {
    public:
        AtomsContextualMenuHandler(AtomsStorage* mapMaster, QWidget* menuParent);

        void undoAlteration();
        void redoAlteration();
        void copySelectedAtomsToClipboard();
        void pasteAtomsFromClipboard();
        void removeSelectedAtoms();
        void moveSelectedAtomsToLayer(int targetLayer);
        void alterSelectedAtomsVisibility(bool isHidden);
        void alterSelectedAtomsAvailability(bool isLocked);

        void invokeMenu(const QVector<RPZAtomId> &toManipulate, const QPoint &whereToDisplay);

    private:
        QWidget* _menuParent = nullptr;
        AtomsStorage* _mapMaster = nullptr;  

        static inline QVector<RPZAtomId> _copyClipboard;
        static inline QVector<RPZAtomId> _latestInvokedAtomIds;
        static inline PossibleActionsOnAtomList _latestPossibleActions;

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

        QAction* _genRemoveAction(bool isEnabled);
        QList<QAction*> _genLayerActions(int riseLayoutTarget, int lowerLayoutTarget);
        QList<QAction*> _genUndoRedoActions(bool canUndo, bool canRedo);
        QList<QAction*> _genCopyPasteActions(bool canCopy, bool canPaste);
        QList<QAction*> _genVisibilityActions(bool areEnabled);
        QList<QAction*> _genAvailabilityActions(bool areEnabled);

        void _addCopyPasteActionsToShortcuts();
        void _addUndoRedoActionsToShortcuts();
};