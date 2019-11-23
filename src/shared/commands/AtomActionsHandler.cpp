#include "AtomActionsHandler.h"

AtomActionsHandler::AtomActionsHandler(AtomSelector* selector, QWidget* parent) : _selector(selector) {
    this->_addCopyPasteActionsToShortcuts(parent);
    this->_addUndoRedoActionsToShortcuts(parent);
    this->_addRemoveActionToShortcuts(parent);
}

const QList<RPZAtom::Id> AtomActionsHandler::fromSelector() {
    return this->_selector->selectedIds();
}

///
///
///

void AtomActionsHandler::undoAlteration() {
    if(!Authorisations::isHostAble()) return;
    QMetaObject::invokeMethod(HintThread::hint(), "undo");
}
void AtomActionsHandler::redoAlteration() {
    if(!Authorisations::isHostAble()) return;
    QMetaObject::invokeMethod(HintThread::hint(), "redo");
}

void AtomActionsHandler::copyToClipboard(const QList<RPZAtom::Id> &ids) {
    if(!Authorisations::isHostAble()) return;
    if(!ids.count()) return;
    Clipboard::set(ids);
}

void AtomActionsHandler::pasteAtomsFromClipboard() {
    
    auto clipboard = Clipboard::get();

    if(!clipboard.count()) return;
    if(!Authorisations::isHostAble()) return;
    
    QMetaObject::invokeMethod(HintThread::hint(), "duplicateAtoms", 
        Q_ARG(QList<RPZAtom::Id>, clipboard)
    );

}

void AtomActionsHandler::removeAtoms(const QList<RPZAtom::Id> &ids) {
    if(!Authorisations::isHostAble()) return;
    if(!ids.count()) return;
    RemovedPayload payload(ids);
    AlterationHandler::get()->queueAlteration(HintThread::hint(), payload);
}

void AtomActionsHandler::moveAtomsToLayer(const QList<RPZAtom::Id> &ids, int targetLayer) {
    if(!Authorisations::isHostAble()) return;
    MetadataChangedPayload payload(ids, {{RPZAtom::Parameter::Layer, targetLayer}});
    AlterationHandler::get()->queueAlteration(HintThread::hint(), payload);

}

void AtomActionsHandler::alterAtomsVisibility(const QList<RPZAtom::Id> &ids, bool hide) {
    if(!Authorisations::isHostAble()) return;
    MetadataChangedPayload payload(ids, {{RPZAtom::Parameter::Hidden, hide}});
    AlterationHandler::get()->queueAlteration(HintThread::hint(), payload);

}

void AtomActionsHandler::alterAtomsAvailability(const QList<RPZAtom::Id> &ids, bool lock) {
    if(!Authorisations::isHostAble()) return;
    MetadataChangedPayload payload(ids, {{RPZAtom::Parameter::Locked, lock}});
    AlterationHandler::get()->queueAlteration(HintThread::hint(), payload);

}

///
///
///

void AtomActionsHandler::_addRemoveActionToShortcuts(QWidget* toAddShortcutsTo) {
    
    auto removeAction = RPZActions::remove();
    QObject::connect(
        removeAction, &QAction::triggered,
        [=]() {
            removeAtoms(this->fromSelector());
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
            pasteAtomsFromClipboard();
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
            undoAlteration();
        }
    );


    auto redoAction = RPZActions::redo();
    QObject::connect(
        redoAction, &QAction::triggered,
        [=]() {
            redoAlteration();
        }
    );

    toAddShortcutsTo->addAction(undoAction);
    toAddShortcutsTo->addAction(redoAction);

}

