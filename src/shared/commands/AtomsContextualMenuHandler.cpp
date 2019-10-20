#include "AtomsContextualMenuHandler.h"

AtomsContextualMenuHandler::AtomsContextualMenuHandler(AtomsStorage* mapMaster, QWidget* menuParent) : 
    _menuParent(menuParent), 
    _mapMaster(mapMaster) { 
        
        //bind actions to shortcuts
        _addCopyPasteActionsToShortcuts();
        _addUndoRedoActionsToShortcuts();

    }

void AtomsContextualMenuHandler::undoAlteration() {
    if(!ClientBindable::isHostAble()) return;
    QMetaObject::invokeMethod(this->_mapMaster, "undo");
}
void AtomsContextualMenuHandler::redoAlteration() {
    if(!ClientBindable::isHostAble()) return;
    QMetaObject::invokeMethod(this->_mapMaster, "redo");
}

void AtomsContextualMenuHandler::copySelectedAtomsToClipboard() {
    if(!ClientBindable::isHostAble()) return;
    _copyClipboard = _latestInvokedAtomIds;
}

void AtomsContextualMenuHandler::pasteAtomsFromClipboard() {
    
    if(!_copyClipboard.count()) return;
    if(!ClientBindable::isHostAble()) return;
    
    QMetaObject::invokeMethod(this->_mapMaster, "duplicateAtoms", 
        Q_ARG(QVector<RPZAtomId>, _copyClipboard)
    );

}

void AtomsContextualMenuHandler::removeSelectedAtoms() {
    if(!ClientBindable::isHostAble()) return;
    RemovedPayload payload(_latestInvokedAtomIds);
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);

}

void AtomsContextualMenuHandler::moveSelectedAtomsToLayer(int targetLayer) {
    if(!ClientBindable::isHostAble()) return;
    MetadataChangedPayload payload(_latestInvokedAtomIds, {{AtomParameter::Layer, targetLayer}});
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);

}

void AtomsContextualMenuHandler::alterSelectedAtomsVisibility(bool isHidden) {
    if(!ClientBindable::isHostAble()) return;
    MetadataChangedPayload payload(_latestInvokedAtomIds, {{AtomParameter::Hidden, isHidden}});
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);

}

void AtomsContextualMenuHandler::alterSelectedAtomsAvailability(bool isLocked) {
    if(!ClientBindable::isHostAble()) return;
    MetadataChangedPayload payload(_latestInvokedAtomIds, {{AtomParameter::Locked, isLocked}});
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);

}

void AtomsContextualMenuHandler::invokeMenu(const QVector<RPZAtomId> &toManipulate, const QPoint &whereToDisplay) {
    
    //get instr
    _latestPossibleActions = ClientBindable::isHostAble() ? 
                                this->_mapMaster->getPossibleActions(toManipulate) :
                                PossibleActionsOnAtomList();
    _latestInvokedAtomIds = toManipulate;
    
    //display menu
    QMenu menu(this->_menuParent);
    
        //copy/paste
        menu.addActions(this->_genCopyPasteActions(_latestPossibleActions.canCopy, _copyClipboard.count()));
        menu.addSeparator();

        //undo/redo 
        menu.addActions(this->_genUndoRedoActions(_latestPossibleActions.somethingUndoable, _latestPossibleActions.somethingRedoable));
        menu.addSeparator();

        //layer modification
        if(_latestPossibleActions.canChangeLayer) {
            auto layerActions = this->_genLayerActions(
                _latestPossibleActions.targetUpLayer, 
                _latestPossibleActions.targetDownLayer
            );
            menu.addActions(layerActions);
            menu.addSeparator();
        }

        //visibility changes
        menu.addActions(this->_genVisibilityActions(_latestPossibleActions.canChangeVisibility));
        menu.addSeparator();

        //availability changes
        menu.addActions(this->_genAvailabilityActions(_latestPossibleActions.canChangeAvailability));
        menu.addSeparator();

        //removal
        menu.addAction(this->_genRemoveAction(_latestPossibleActions.canRemove));

    menu.exec(whereToDisplay);
}


QAction* AtomsContextualMenuHandler::_genRemoveAction(bool isEnabled) {
    
    if(!this->_removeAction) {
        this->_removeAction = RPZActions::remove();
        QObject::connect(
            this->_removeAction, &QAction::triggered,
            [=]() {this->removeSelectedAtoms();}
        );
    }

    this->_removeAction->setEnabled(isEnabled);

    return this->_removeAction;
}

QList<QAction*> AtomsContextualMenuHandler::_genLayerActions(int riseLayoutTarget, int lowerLayoutTarget) {

    if(this->_riseAction) delete this->_riseAction;
    if(this->_lowerAction) delete this->_lowerAction;

    this->_riseAction = RPZActions::raiseAtom(riseLayoutTarget);
    this->_lowerAction = RPZActions::lowerAtom(lowerLayoutTarget);

    QObject::connect(
        this->_riseAction, &QAction::triggered,
        [=]() {this->moveSelectedAtomsToLayer(riseLayoutTarget);}
    );
    
    QObject::connect(
        this->_lowerAction, &QAction::triggered,
        [=]() {this->moveSelectedAtomsToLayer(lowerLayoutTarget);}
    );

    QList<QAction*> layerActions;    
    layerActions.append(this->_riseAction);
    layerActions.append(this->_lowerAction);
    return layerActions;

}

QList<QAction*> AtomsContextualMenuHandler::_genUndoRedoActions(bool canUndo, bool canRedo) { 

    if(!this->_undoAction) {
        this->_undoAction = RPZActions::undo();
        QObject::connect(
            this->_undoAction, &QAction::triggered,
            [=]() {this->undoAlteration();}
        );
    }

    if(!this->_redoAction) {
        this->_redoAction = RPZActions::redo();
        QObject::connect(
            this->_redoAction, &QAction::triggered,
            [=]() {this->redoAlteration();}
        );
    }

    this->_undoAction->setEnabled(canUndo);
    this->_redoAction->setEnabled(canRedo);

    QList<QAction*> out;
    out.append(this->_undoAction);
    out.append(this->_redoAction);
    return out;
}

void AtomsContextualMenuHandler::_addCopyPasteActionsToShortcuts() {
    auto actions = this->_genCopyPasteActions(true, true);
    this->_copyAction = nullptr;
    this->_pasteAction = nullptr;
    this->_menuParent->addActions(actions);
}

void AtomsContextualMenuHandler::_addUndoRedoActionsToShortcuts() {
    auto actions = this->_genUndoRedoActions(true, true);
    this->_undoAction = nullptr;
    this->_redoAction = nullptr;
    this->_menuParent->addActions(actions);
}

QList<QAction*> AtomsContextualMenuHandler::_genCopyPasteActions(bool canCopy, bool canPaste) {
    
    if(!this->_copyAction) {
        this->_copyAction = RPZActions::copy();
        QObject::connect(
            this->_copyAction, &QAction::triggered,
            [=]() {this->copySelectedAtomsToClipboard();}
        );
    }

    if(!this->_pasteAction) {
        this->_pasteAction = RPZActions::paste();
        QObject::connect(
            this->_pasteAction, &QAction::triggered,
            [=]() {this->pasteAtomsFromClipboard();}
        );
    }

    this->_copyAction->setEnabled(canCopy);
    this->_pasteAction->setEnabled(canPaste);

    QList<QAction*> out;
    out.append(this->_copyAction);
    out.append(this->_pasteAction);
    return out;
}

QList<QAction*> AtomsContextualMenuHandler::_genVisibilityActions(bool areEnabled) {     
    
    if(!this->_showAction) {
        this->_showAction = RPZActions::showAtom();
        QObject::connect(
            this->_showAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsVisibility(false);}
        );
    }
    
    if(!this->_hideAction) {
        this->_hideAction = RPZActions::hideAtom();
        QObject::connect(
            this->_hideAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsVisibility(true);}
        );
    }

    this->_showAction->setEnabled(areEnabled);
    this->_hideAction->setEnabled(areEnabled);
    
    QList<QAction*> out;
    out.append(this->_showAction);
    out.append(this->_hideAction);
    return out;
}

QList<QAction*> AtomsContextualMenuHandler::_genAvailabilityActions(bool areEnabled) {
    
    if(!this->_lockAction) {
        this->_lockAction = RPZActions::lockAtom();
        QObject::connect(
            this->_lockAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsAvailability(true);}
        );
    }
    
    if(!this->_unlockAction) {
        this->_unlockAction = RPZActions::unlockAtom();
        QObject::connect(
            this->_unlockAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsAvailability(false);}
        );
    }

    this->_lockAction->setEnabled(areEnabled);
    this->_unlockAction->setEnabled(areEnabled);

    QList<QAction*> out;
    out.append(this->_lockAction);
    out.append(this->_unlockAction);
    return out;
}
