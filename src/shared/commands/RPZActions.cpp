#include "RPZActions.h"

QAction* RPZActions::redo() {
    auto action = new QAction("Rétablir");
    action->setShortcut(QKeySequence::Redo);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::undo() {
    auto action = new QAction("Annuler");
    action->setShortcut(QKeySequence::Undo);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::copy() {
    auto action = new QAction("Copier");
    action->setShortcut(QKeySequence::Copy);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::paste() {
    auto action = new QAction("Coller");
    action->setShortcut(QKeySequence::Paste);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::createFolder() {
    return new QAction(
        QIcon(":/icons/app/manager/folder.png"), 
        "Créer un dossier"
    );
}

QAction* RPZActions::remove() {
    auto action = new QAction(
        QIcon(":/icons/app/tools/bin.png"), 
        "Supprimer"
    );
    action->setShortcut(QKeySequence::Delete);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::raiseAtom(int targetLayer) {
    auto descr = QString("Remonter (Calque %1)")
                    .arg(QString::number(targetLayer));

    return new QAction(
        QIcon(":/icons/app/tools/raise.png"),
        descr
    );
}

QAction* RPZActions::lowerAtom(int targetLayer) {
    auto descr = QString("Descendre (Calque %1)")
                    .arg(QString::number(targetLayer));

    return new QAction(
        QIcon(":/icons/app/tools/lower.png"),
        descr
    );
}

QAction* RPZActions::showAtom() {
    return new QAction(
        QIcon(":/icons/app/tools/shown.png"),
        "Montrer"
    );
}

QAction* RPZActions::hideAtom() {
    return new QAction(
        QIcon(":/icons/app/tools/hidden.png"),
        "Cacher"
    );
}

QAction* RPZActions::resetView() {
    auto action = new QAction;
    action->setIcon(QIcon(":/icons/app/tools/reset.png"));
    action->setIconText("Réinitialiser la vue");
    return action;
}

QAction* RPZActions::lockAtom() {
    return new QAction(
        QIcon(":/icons/app/tools/lock.png"),
        "Verouiller"
    );
}

QAction* RPZActions::unlockAtom() {
    return new QAction(
        QIcon(":/icons/app/tools/unlock.png"),
        "Déverouiller"
    );
}

QAction* RPZActions::quit() {
    auto action = new QAction(
        QIcon(":/icons/app/tools/exit.png"),
        I18n::tr()->Menu_Quit()
    );
    action->setShortcut(QKeySequence::Close);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::patchnote() {
    return new QAction(I18n::tr()->Menu_Patchnotes(APP_FULL_DENOM));
}

QAction* RPZActions::checkUpdates() {
    return new QAction(I18n::tr()->Menu_CheckForUpgrades());
}

QAction* RPZActions::openInternalDataFolder() {
    return new QAction(I18n::tr()->Menu_OpenDataFolder(AppContext::getAppDataLocation()));
}

QAction* RPZActions::openLatestLog() {
    return new QAction(I18n::tr()->Menu_OpenLatestLog());
}

QAction* RPZActions::openFullLog() {
    return new QAction(I18n::tr()->Menu_OpenLog());
}

QAction* RPZActions::openMaintenanceTool() {
    return new QAction(I18n::tr()->Menu_OpenMaintenanceTool());
}

QAction* RPZActions::loadMap() {
    auto action = new QAction("Charger une carte");
    action->setShortcut(QKeySequence::Open);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::saveMap() {
    auto action = new QAction("Sauvegarder la carte");
    action->setShortcut(QKeySequence::Save);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::saveAsMap() {
    auto action = new QAction("Enregistrer la carte sous...");
    action->setShortcut(QKeySequence::SaveAs);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}
