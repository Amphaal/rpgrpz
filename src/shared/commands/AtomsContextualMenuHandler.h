#pragma once

#include "src/shared/atoms/AtomsStorage.h"
#include "src/shared/commands/RPZActions.h"
#include "src/shared/payloads/Payloads.h"

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

        void invokeMenu(int topMostLayer, int bottomMostLayer, int countAtoms, const QPoint &whereToDisplay);

    private:
        QWidget* _menuParent = nullptr;
        AtomsStorage* _mapMaster = nullptr;  

        static inline QVector<snowflake_uid> _copyClipboard;

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

        QAction* _genRemoveAction(int selectedAtoms);
        QList<QAction*> _genLayerActions(int riseLayoutTarget, int lowerLayoutTarget, int selectedAtoms);
        QList<QAction*> _genUndoRedoActions();
        QList<QAction*> _genCopyPasteActions();
        QList<QAction*> _genVisibilityActions(int selectedAtoms);
        QList<QAction*> _genAvailabilityActions(int selectedAtoms);
};