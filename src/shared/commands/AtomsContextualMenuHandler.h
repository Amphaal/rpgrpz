#pragma once

#include "src/shared/hints/AtomsStorage.h"
#include "src/shared/commands/RPZActions.h"
#include "src/shared/payloads/Payloads.h"

#include "src/ui/_others/ConnectivityObserver.h"

#include <QAction>
#include <QWidget>
#include <QMenu>

#include <QMetaObject>

#include "AtomActionsHandler.h"

class AtomsContextualMenuHandler {
    public:
        AtomsContextualMenuHandler(AtomsStorage* mapMaster, QWidget* menuParent);

        void invokeMenu(const QVector<RPZAtom::Id> &toManipulate, const QPoint &whereToDisplay);

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
        AtomsStorage* _mapMaster = nullptr;  

        QVector<RPZAtom::Id> _latestInvokedAtomIds;
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