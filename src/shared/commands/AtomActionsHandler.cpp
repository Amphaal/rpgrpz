#include "AtomActionsHandler.h"

AtomActionsHandler::AtomActionsHandler(AtomsStorage* master, AtomSelector* selector, QWidget* parent) : 
    _selector(selector), 
    _mapMaster(master) {

    this->_addCopyPasteActionsToShortcuts(parent);
    this->_addUndoRedoActionsToShortcuts(parent);
    this->_addRemoveActionToShortcuts(parent);

}

const QVector<RPZAtom::Id> AtomActionsHandler::fromSelector() {
    return this->_selector->selectedIds();
}

///
///
///

void AtomActionsHandler::undoAlteration(AtomsStorage* master) {
    if(!ConnectivityObserver::isHostAble()) return;
    QMetaObject::invokeMethod(master, "undo");
}
void AtomActionsHandler::redoAlteration(AtomsStorage* master) {
    if(!ConnectivityObserver::isHostAble()) return;
    QMetaObject::invokeMethod(master, "redo");
}

void AtomActionsHandler::copyToClipboard(const QVector<RPZAtom::Id> &ids) {
    if(!ConnectivityObserver::isHostAble()) return;
    if(!ids.count()) return;
    Clipboard::set(ids);
}

void AtomActionsHandler::pasteAtomsFromClipboard(AtomsStorage* master) {
    
    auto clipboard = Clipboard::get();

    if(!clipboard.count()) return;
    if(!ConnectivityObserver::isHostAble()) return;
    
    QMetaObject::invokeMethod(master, "duplicateAtoms", 
        Q_ARG(QVector<RPZAtom::Id>, clipboard)
    );

}

void AtomActionsHandler::removeAtoms(AtomsStorage* master, const QVector<RPZAtom::Id> &ids) {
    if(!ConnectivityObserver::isHostAble()) return;
    RemovedPayload payload(ids);
    AlterationHandler::get()->queueAlteration(master, payload);
}

void AtomActionsHandler::moveAtomsToLayer(AtomsStorage* master, const QVector<RPZAtom::Id> &ids, int targetLayer) {
    if(!ConnectivityObserver::isHostAble()) return;
    MetadataChangedPayload payload(ids, {{RPZAtom::Parameter::Layer, targetLayer}});
    AlterationHandler::get()->queueAlteration(master, payload);

}

void AtomActionsHandler::alterAtomsVisibility(AtomsStorage* master, const QVector<RPZAtom::Id> &ids, bool hide) {
    if(!ConnectivityObserver::isHostAble()) return;
    MetadataChangedPayload payload(ids, {{RPZAtom::Parameter::Hidden, hide}});
    AlterationHandler::get()->queueAlteration(master, payload);

}

void AtomActionsHandler::alterAtomsAvailability(AtomsStorage* master, const QVector<RPZAtom::Id> &ids, bool lock) {
    if(!ConnectivityObserver::isHostAble()) return;
    MetadataChangedPayload payload(ids, {{RPZAtom::Parameter::Locked, lock}});
    AlterationHandler::get()->queueAlteration(master, payload);

}

///
///
///


void AtomActionsHandler::_addRemoveActionToShortcuts(QWidget* toAddShortcutsTo) {
    
    auto removeAction = RPZActions::remove();
    QObject::connect(
        removeAction, &QAction::triggered,
        [=]() {
            removeAtoms(this->_mapMaster, this->fromSelector());
        }
    );

    toAddShortcutsTo->addAction(removeAction);

}

void AtomActionsHandler::_addCopyPasteActionsToShortcuts(QWidget* toAddShortcutsTo) {
        
    auto copyAction = RPZActions::copy();
    QObject::connect(
        copyAction, &QAction::triggered,
        [=]() {
            copyToClipboard(this->fromSelector());
        }
    );


    auto pasteAction = RPZActions::paste();
    QObject::connect(
        pasteAction, &QAction::triggered,
        [=]() {
            pasteAtomsFromClipboard(this->_mapMaster);
        }
    );

    toAddShortcutsTo->addAction(copyAction);
    toAddShortcutsTo->addAction(pasteAction);

}

void AtomActionsHandler::_addUndoRedoActionsToShortcuts(QWidget* toAddShortcutsTo) {

    auto undoAction = RPZActions::undo();
    QObject::connect(
        undoAction, &QAction::triggered,
        [=]() {
            undoAlteration(this->_mapMaster);
        }
    );


    auto redoAction = RPZActions::redo();
    QObject::connect(
        redoAction, &QAction::triggered,
        [=]() {
            redoAlteration(this->_mapMaster);
        }
    );

    toAddShortcutsTo->addAction(undoAction);
    toAddShortcutsTo->addAction(redoAction);

}

