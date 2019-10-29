#include "AtomsContextualMenuHandler.h"

AtomsContextualMenuHandler::AtomsContextualMenuHandler(AtomsStorage* mapMaster, QWidget* menuParent) :  _menuParent(menuParent), _mapMaster(mapMaster) {}

///
///
///

void AtomsContextualMenuHandler::_moveAtomsToLayer(int layer) {
    AtomActionsHandler::moveAtomsToLayer(
        this->_mapMaster, 
        this->_latestInvokedAtomIds,
        layer
    );
}

void AtomsContextualMenuHandler::_removeAtoms() {
    AtomActionsHandler::removeAtoms(
        this->_mapMaster, 
        this->_latestInvokedAtomIds
    );
}

void AtomsContextualMenuHandler::_undo() {
    AtomActionsHandler::undoAlteration(this->_mapMaster);
}

void AtomsContextualMenuHandler::_redo() {
    AtomActionsHandler::redoAlteration(this->_mapMaster);
}

void AtomsContextualMenuHandler::_copy() {
    AtomActionsHandler::copyToClipboard(this->_latestInvokedAtomIds);
}

void AtomsContextualMenuHandler::_paste() {
    AtomActionsHandler::pasteAtomsFromClipboard(this->_mapMaster);
}

void AtomsContextualMenuHandler::_setVisibility(bool hide) {
    AtomActionsHandler::alterAtomsVisibility(
        this->_mapMaster, 
        this->_latestInvokedAtomIds,
        hide
    );
}

void AtomsContextualMenuHandler::_setAvailability(bool lock) {
    AtomActionsHandler::alterAtomsAvailability(
        this->_mapMaster, 
        this->_latestInvokedAtomIds,
        lock
    );
}

///
///
///

void AtomsContextualMenuHandler::invokeMenu(const QVector<RPZAtom::Id> &toManipulate, const QPoint &whereToDisplay) {

    //get instr
    this->_latestPossibleActions = ConnectivityObserver::isHostAble() ? this->_mapMaster->getPossibleActions(toManipulate) : PossibleActionsOnAtomList();
    this->_latestInvokedAtomIds = toManipulate;
    
    //display menu
    QMenu menu(this->_menuParent);
    
        //copy/paste
        menu.addActions(this->_genCopyPasteActions());
        menu.addSeparator();

        //undo/redo 
        menu.addActions(this->_genUndoRedoActions());
        menu.addSeparator();

        //layer modification
        if(this->_latestPossibleActions.canChangeLayer) {
            auto layerActions = this->_genLayerActions();
            menu.addActions(layerActions);
            menu.addSeparator();
        }

        //visibility changes
        menu.addActions(this->_genVisibilityActions());
        menu.addSeparator();

        //availability changes
        menu.addActions(this->_genAvailabilityActions());
        menu.addSeparator();

        //removal
        menu.addAction(this->_genRemoveAction());

    menu.exec(whereToDisplay);
}


QAction* AtomsContextualMenuHandler::_genRemoveAction() {
    
    if(!this->_removeAction) {
        this->_removeAction = RPZActions::remove();
        QObject::connect(
            this->_removeAction, &QAction::triggered,
            [=]() {this->_removeAtoms();}
        );
    }

    this->_removeAction->setEnabled(this->_latestPossibleActions.canRemove);

    return this->_removeAction;
}

QList<QAction*> AtomsContextualMenuHandler::_genLayerActions() {

    auto riseLayoutTarget = this->_latestPossibleActions.targetUpLayer;
    auto lowerLayoutTarget = this->_latestPossibleActions.targetDownLayer;

    if(this->_riseAction) delete this->_riseAction;
    if(this->_lowerAction) delete this->_lowerAction;

    this->_riseAction = RPZActions::raiseAtom(riseLayoutTarget);
    this->_lowerAction = RPZActions::lowerAtom(lowerLayoutTarget);

    QObject::connect(
        this->_riseAction, &QAction::triggered,
        [=]() {this->_moveAtomsToLayer(riseLayoutTarget);}
    );
    
    QObject::connect(
        this->_lowerAction, &QAction::triggered,
        [=]() {this->_moveAtomsToLayer(lowerLayoutTarget);}
    );

    return { this->_riseAction, this->_lowerAction };

}

QList<QAction*> AtomsContextualMenuHandler::_genUndoRedoActions() { 

    if(!this->_undoAction) {
        this->_undoAction = RPZActions::undo();
        QObject::connect(
            this->_undoAction, &QAction::triggered,
            [=]() {this->_undo();}
        );
    }

    if(!this->_redoAction) {
        this->_redoAction = RPZActions::redo();
        QObject::connect(
            this->_redoAction, &QAction::triggered,
            [=]() {this->_redo();}
        );
    }
 
    this->_undoAction->setEnabled(this->_latestPossibleActions.somethingUndoable);
    this->_redoAction->setEnabled(this->_latestPossibleActions.somethingRedoable);

    return { this->_undoAction, this->_redoAction };
}

QList<QAction*> AtomsContextualMenuHandler::_genCopyPasteActions() {
    
    if(!this->_copyAction) {
        this->_copyAction = RPZActions::copy();
        QObject::connect(
            this->_copyAction, &QAction::triggered,
            [=]() {this->_copy();}
        );
    }

    if(!this->_pasteAction) {
        this->_pasteAction = RPZActions::paste();
        QObject::connect(
            this->_pasteAction, &QAction::triggered,
            [=]() {this->_paste();}
        );
    }

    this->_copyAction->setEnabled(this->_latestPossibleActions.canCopy);
    this->_pasteAction->setEnabled(Clipboard::get().count());

    return { this->_copyAction, this->_pasteAction };

}

QList<QAction*> AtomsContextualMenuHandler::_genVisibilityActions() {     
    
    if(!this->_showAction) {
        this->_showAction = RPZActions::showAtom();
        QObject::connect(
            this->_showAction, &QAction::triggered,
            [=]() {this->_setVisibility(false);}
        );
    }
    
    if(!this->_hideAction) {
        this->_hideAction = RPZActions::hideAtom();
        QObject::connect(
            this->_hideAction, &QAction::triggered,
            [=]() {this->_setVisibility(true);}
        );
    }

    this->_showAction->setEnabled(this->_latestPossibleActions.canChangeVisibility);
    this->_hideAction->setEnabled(this->_latestPossibleActions.canChangeVisibility);
    
    return { this->_showAction, this->_hideAction };

}

QList<QAction*> AtomsContextualMenuHandler::_genAvailabilityActions() {
    
    if(!this->_lockAction) {
        this->_lockAction = RPZActions::lockAtom();
        QObject::connect(
            this->_lockAction, &QAction::triggered,
            [=]() {this->_setAvailability(true);}
        );
    }
    
    if(!this->_unlockAction) {
        this->_unlockAction = RPZActions::unlockAtom();
        QObject::connect(
            this->_unlockAction, &QAction::triggered,
            [=]() {this->_setAvailability(false);}
        );
    }

    this->_lockAction->setEnabled(this->_latestPossibleActions.canChangeAvailability);
    this->_unlockAction->setEnabled(this->_latestPossibleActions.canChangeAvailability);

    return { this->_lockAction, this->_unlockAction };

}
