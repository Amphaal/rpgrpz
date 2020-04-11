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
// different license and copyright still refer to this GNU General Public License.

#include "AtomsContextualMenuHandler.h"

AtomsContextualMenuHandler::AtomsContextualMenuHandler(QWidget* menuParent) :  _menuParent(menuParent) {}

///
///
///

void AtomsContextualMenuHandler::_moveAtomsToLayer(int layer) {
    AtomActionsHandler::moveAtomsToLayer(
        this->_latestInvokedAtomIds,
        layer
    );
}

void AtomsContextualMenuHandler::_removeAtoms() {
    AtomActionsHandler::removeAtoms(
        this->_latestInvokedAtomIds
    );
}

void AtomsContextualMenuHandler::_undo() {
    AtomActionsHandler::undoAlteration();
}

void AtomsContextualMenuHandler::_redo() {
    AtomActionsHandler::redoAlteration();
}

void AtomsContextualMenuHandler::_copy() {
    AtomActionsHandler::copyToClipboard(this->_latestInvokedAtomIds);
}

void AtomsContextualMenuHandler::_paste() {
    AtomActionsHandler::pasteAtomsFromClipboard();
}

void AtomsContextualMenuHandler::_setVisibility(bool hide) {
    AtomActionsHandler::alterAtomsVisibility(
        this->_latestInvokedAtomIds,
        hide
    );
}

void AtomsContextualMenuHandler::_setAvailability(bool lock) {
    AtomActionsHandler::alterAtomsAvailability(
        this->_latestInvokedAtomIds,
        lock
    );
}

///
///
///

void AtomsContextualMenuHandler::invokeMenu(const QList<RPZAtom::Id> &toManipulate, const QPoint &whereToDisplay) {

    //get instr
    this->_latestPossibleActions = HintThread::hint()->getPossibleActions(toManipulate);
    this->_latestInvokedAtomIds = toManipulate;
    
    //display menu
    QMenu menu(this->_menuParent);
    
        //selectionCount 
        auto selectedCountAction = menu.addAction(QObject::tr("%1 selected atom(s)").arg(this->_latestInvokedAtomIds.count()));
        selectedCountAction->setDisabled(true);
        menu.addSeparator();

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
