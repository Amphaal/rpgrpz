#include "AtomsContextualMenuHandler.h"

AtomsContextualMenuHandler::AtomsContextualMenuHandler(AtomsStorage* mapMaster, QWidget* menuParent) : 
    _menuParent(menuParent), 
    _mapMaster(mapMaster) { 

    menuParent->addActions(this->_genCopyPasteActions());
    menuParent->addActions(this->_genUndoRedoActions());

}

void AtomsContextualMenuHandler::undoAlteration() {
    QMetaObject::invokeMethod(this->_mapMaster, "undo");
}
void AtomsContextualMenuHandler::redoAlteration() {
    QMetaObject::invokeMethod(this->_mapMaster, "redo");
}

void AtomsContextualMenuHandler::copySelectedAtomsToClipboard() {
    _copyClipboard = this->_mapMaster->bufferedSelectedAtomIds();
}

void AtomsContextualMenuHandler::pasteAtomsFromClipboard() {
    if(!_copyClipboard.count()) return;
    QMetaObject::invokeMethod(this->_mapMaster, "duplicateAtoms", Q_ARG(QVector<RPZAtomId>, _copyClipboard));
}

void AtomsContextualMenuHandler::removeSelectedAtoms() {
    auto selectedIds = this->_mapMaster->bufferedSelectedAtomIds();
    RemovedPayload payload(selectedIds);
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);
}

void AtomsContextualMenuHandler::moveSelectedAtomsToLayer(int targetLayer) {
    auto selectedIds = this->_mapMaster->bufferedSelectedAtomIds();
    MetadataChangedPayload payload(selectedIds, AtomParameter::Layer, targetLayer);
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);
}

void AtomsContextualMenuHandler::alterSelectedAtomsVisibility(bool isHidden) {
    auto selectedIds = this->_mapMaster->bufferedSelectedAtomIds();
    MetadataChangedPayload payload(selectedIds, AtomParameter::Hidden, isHidden);
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);
}

void AtomsContextualMenuHandler::alterSelectedAtomsAvailability(bool isLocked) {
    auto selectedIds = this->_mapMaster->bufferedSelectedAtomIds();
    MetadataChangedPayload payload(selectedIds, AtomParameter::Locked, isLocked);
    AlterationHandler::get()->queueAlteration(this->_mapMaster, payload);
}

void AtomsContextualMenuHandler::invokeMenu(int topMostLayer, int bottomMostLayer, int countAtoms, const QPoint &whereToDisplay) {
    
    //display menu
    QMenu menu(this->_menuParent);

    menu.addActions(this->_genCopyPasteActions());
    menu.addSeparator();
    menu.addActions(this->_genLayerActions(topMostLayer, bottomMostLayer, countAtoms));
    menu.addSeparator();
    menu.addActions(this->_genVisibilityActions(countAtoms));
    menu.addSeparator();
    menu.addActions(this->_genAvailabilityActions(countAtoms));
    menu.addSeparator();
    menu.addAction(this->_genRemoveAction(countAtoms));

    menu.exec(whereToDisplay);
}


QAction* AtomsContextualMenuHandler::_genRemoveAction(int selectedAtoms) {
    
    if(!this->_removeAction) {
        this->_removeAction = RPZActions::remove();
        QObject::connect(
            this->_removeAction, &QAction::triggered,
            [=]() {this->removeSelectedAtoms();}
        );
    }

    this->_removeAction->setEnabled(selectedAtoms);

    return this->_removeAction;
}

QList<QAction*> AtomsContextualMenuHandler::_genLayerActions(int riseLayoutTarget, int lowerLayoutTarget, int selectedAtoms) {

    if(this->_riseAction) delete this->_riseAction;

    this->_riseAction = RPZActions::raiseAtom(riseLayoutTarget);
    QObject::connect(
        this->_riseAction, &QAction::triggered,
        [=]() {this->moveSelectedAtomsToLayer(riseLayoutTarget);}
    );

    this->_riseAction->setEnabled(selectedAtoms);


    if(this->_lowerAction) delete this->_lowerAction;

    this->_lowerAction = RPZActions::lowerAtom(lowerLayoutTarget);
    QObject::connect(
        this->_lowerAction, &QAction::triggered,
        [=]() {this->moveSelectedAtomsToLayer(lowerLayoutTarget);}
    );

    this->_lowerAction->setEnabled(selectedAtoms);


    QList<QAction*> layerActions;    
    layerActions.append(this->_riseAction);
    layerActions.append(this->_lowerAction);
    return layerActions;

}

QList<QAction*> AtomsContextualMenuHandler::_genUndoRedoActions() { 

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

    QList<QAction*> out;
    out.append(this->_undoAction);
    out.append(this->_redoAction);
    return out;
}

QList<QAction*> AtomsContextualMenuHandler::_genCopyPasteActions() {
    
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

    QList<QAction*> out;
    out.append(this->_copyAction);
    out.append(this->_pasteAction);
    return out;
}

QList<QAction*> AtomsContextualMenuHandler::_genVisibilityActions(int selectedAtoms) {     
    
    if(!this->_showAction) {
        this->_showAction = RPZActions::showAtom();
        QObject::connect(
            this->_showAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsVisibility(false);}
        );
    }
    this->_showAction->setEnabled(selectedAtoms);
    
    if(!this->_hideAction) {
        this->_hideAction = RPZActions::hideAtom();
        QObject::connect(
            this->_hideAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsVisibility(true);}
        );
    }
    this->_hideAction->setEnabled(selectedAtoms);
    
    QList<QAction*> out;
    out.append(this->_showAction);
    out.append(this->_hideAction);
    return out;
}

QList<QAction*> AtomsContextualMenuHandler::_genAvailabilityActions(int selectedAtoms) {
    
    if(!this->_lockAction) {
        this->_lockAction = RPZActions::lockAtom();
        QObject::connect(
            this->_lockAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsAvailability(true);}
        );
    }
    this->_lockAction->setEnabled(selectedAtoms);
    
    if(!this->_unlockAction) {
        this->_unlockAction = RPZActions::unlockAtom();
        QObject::connect(
            this->_unlockAction, &QAction::triggered,
            [=]() {this->alterSelectedAtomsAvailability(false);}
        );
    }
    this->_unlockAction->setEnabled(selectedAtoms);

    QList<QAction*> out;
    out.append(this->_lockAction);
    out.append(this->_unlockAction);
    return out;
}
