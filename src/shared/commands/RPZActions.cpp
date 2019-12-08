#include "RPZActions.h"

QAction* RPZActions::usePingTool() {
    
    auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/ping.png")), 
        QObject::tr("Notify interest point to players")
    );
    
    action->setCheckable(true);
    action->setEnabled(false);
    action->setData((int)MapTool::Ping);

    return action;
}

QAction* RPZActions::useMeasurementTool() {
    
    auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/measuring.png")), 
        QObject::tr("Measure distances")
    );

    action->setCheckable(true);
    action->setEnabled(false);
    action->setData((int)MapTool::Measure);

    return action;

}

QAction* RPZActions::useQuickDrawTool() {
    
    auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/pencil.png")), 
        QObject::tr("Quick draw")
    );

    action->setCheckable(true);
    action->setEnabled(false);
    action->setData((int)MapTool::QuickDraw);

    return action;

}

QAction* RPZActions::activateMinimap() {
   auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/minimap.png")), 
        QObject::tr("Display minimap")
    );

    action->setData("minimap");
    action->setCheckable(true);
    action->setChecked(AppContext::settings()->minimapActive());

    return action;
}

QAction* RPZActions::activateGridIndicator() {
    
    auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/grid.png")), 
        QObject::tr("Display grid indicators")
    );

    action->setData("grid");
    action->setCheckable(true);
    action->setChecked(AppContext::settings()->gridActive());

    QObject::connect(
        action, &QAction::triggered,
        [=](auto checked) {
            AppContext::settings()->setValue(
                action->data().toString(), 
                checked
            );
        }
    );

    return action;

}
QAction* RPZActions::activateScaleIndicator() {
    
    auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/ruler.png")), 
        QObject::tr("Display scale indicators")
    );

    action->setData("scale");
    action->setCheckable(true);
    action->setChecked(AppContext::settings()->scaleActive());

    QObject::connect(
        action, &QAction::triggered,
        [=](auto checked) {
            AppContext::settings()->setValue(
                action->data().toString(), 
                checked
            );
        }
    );

    return action;

}

QAction* RPZActions::redo() {
    auto action = new QAction(QObject::tr("Redo"));
    action->setShortcut(QKeySequence::Redo);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::undo() {
    auto action = new QAction(QObject::tr("Undo"));
    action->setShortcut(QKeySequence::Undo);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::copy() {
    auto action = new QAction(QObject::tr("Copy"));
    action->setShortcut(QKeySequence::Copy);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::paste() {
    auto action = new QAction(QObject::tr("Paste"));
    action->setShortcut(QKeySequence::Paste);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::createFolder() {
    return new QAction(
        QIcon(QStringLiteral(u":/icons/app/manager/folder.png")), 
        QObject::tr("Create a folder")
    );
}

QAction* RPZActions::remove() {
    auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/bin.png")), 
        QObject::tr("Remove")
    );
    action->setShortcut(QKeySequence::Delete);
    action->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::raiseAtom(int targetLayer) {
    auto descr = QObject::tr("Raise (Layer %1)");
    descr = descr.arg(QString::number(targetLayer));

    return new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/raise.png")),
        descr
    );
}

QAction* RPZActions::lowerAtom(int targetLayer) {
    auto descr = QObject::tr("Lower (Layer %1)");
    descr = descr.arg(QString::number(targetLayer));

    return new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/lower.png")),
        descr
    );
}

QAction* RPZActions::showAtom() {
    return new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/shown.png")),
        QObject::tr("Show")
    );
}

QAction* RPZActions::hideAtom() {
    return new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/hidden.png")),
        QObject::tr("Hide")
    );
}

QAction* RPZActions::resetView() {
    auto action = new QAction;
    action->setIcon(QIcon(QStringLiteral(u":/icons/app/tools/reset_view.png")));
    action->setIconText(QObject::tr("Reset the view"));
    action->setData((int)MapAction::ResetView);
    return action;
}

QAction* RPZActions::resetTool() {
    auto action = new QAction;
    action->setIcon(QIcon(QStringLiteral(u":/icons/app/tools/cursor.png")));
    action->setIconText(QObject::tr("Back to Select tool"));
    action->setData((int)MapAction::ResetTool);
    return action;
}

QAction* RPZActions::lockAtom() {
    return new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/lock.png")),
        QObject::tr("Lock")
    );
}

QAction* RPZActions::unlockAtom() {
    return new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/unlock.png")),
        QObject::tr("Unlock")
    );
}

QAction* RPZActions::quit() {
    auto action = new QAction(
        QIcon(QStringLiteral(u":/icons/app/tools/exit.png")),
        QObject::tr("Quit")
    );
    action->setShortcut(QKeySequence::Close);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::patchnote() {
    return new QAction(
        QObject::tr("%1 - Patch Notes")
            .arg(APP_FULL_DENOM)
    );
}

QAction* RPZActions::sentry() {
    return new QAction(
        QObject::tr("Test Sentry bug reporting")
    );
}

QAction* RPZActions::checkUpdates() {
    return new QAction(QObject::tr("Check for updates..."));
}

QAction* RPZActions::openInternalDataFolder() {
    return new QAction(
        QObject::tr("Open application data folder (%1)")
            .arg(AppContext::getAppDataLocation())
    );
}

QAction* RPZActions::openLatestLog() {
    return new QAction(QObject::tr("Show session log"));
}

QAction* RPZActions::openFullLog() {
    return new QAction(QObject::tr("Show full log"));
}

QAction* RPZActions::openMaintenanceTool() {
    return new QAction(QObject::tr("Launch maintenance tool"));
}

QAction* RPZActions::loadAMap() {
    auto action = new QAction(QObject::tr("Load a map"));
    action->setShortcut(QKeySequence::Open);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::createANewMap() {
    auto action = new QAction(QObject::tr("Create a new map"));
    action->setShortcut(QKeySequence::New);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::saveTheMap() {
    auto action = new QAction(QObject::tr("Save map"));
    action->setShortcut(QKeySequence::Save);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}

QAction* RPZActions::saveTheMapAs() {
    auto action = new QAction(QObject::tr("Save as map..."));
    action->setShortcut(QKeySequence::SaveAs);
    action->setShortcutVisibleInContextMenu(true);
    return action;
}
