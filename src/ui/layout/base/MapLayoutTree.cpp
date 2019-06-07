#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(QWidget * parent) : RPZTree(parent) {
    
    this->setItemDelegateForColumn(1, new LockAndVisibilityDelegate);
    this->setItemDelegateForColumn(2, new OwnerDelegate);

    this->setColumnCount(3);

    this->setHeaderHidden(true);
    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);

    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    //context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(
        this, &QWidget::customContextMenuRequested,
        this, &MapLayoutTree::_renderCustomContextMenu
    );

 
}

void MapLayoutTree::_renderCustomContextMenu(const QPoint &pos) {
    
    auto itemsToProcess = this->selectedItems();

    //check selected items (autoselected on right click)
    if(!itemsToProcess.count()) {
        //get elem under cursor
        auto item = this->itemAt(pos);
        if(item) {
            //prevent usage of layer item
            if(item->parent()) {
                itemsToProcess.append(item);
                item->setSelected(true);
            }
        }
    }

    //create menu
    this->_generateMenu(itemsToProcess, this->viewport()->mapToGlobal(pos));
}

QList<QAction*> MapLayoutTree::_genLayerActions(QList<QTreeWidgetItem*> &selectedItems) {
    
    //targets
    auto riseLayoutTarget = selectedItems.first()->parent()->data(0, Qt::UserRole).toInt() + 1;
    auto lowerLayoutTarget = selectedItems.last()->parent()->data(0, Qt::UserRole).toInt() - 1;

    //helper
    auto _moveSelectionToLayer = [&](int targetLayer) {

        auto selectedIds = this->_selectedAtomIds();

        //unilateral event, expect only outer calls
        this->_hints->handleAlterationRequest(LayerChangedPayload(selectedIds, targetLayer));
    };

    //list of layer actions to bind to menu
    QList<QAction*> layerActions;
    
        //rise...
        auto riseAction = RPZActions::raiseAtom(riseLayoutTarget);
        QObject::connect(
            riseAction, &QAction::triggered,
            [=]() {_moveSelectionToLayer(riseLayoutTarget);}
        );
        layerActions.append(riseAction);

        //lower...
        auto lowerAction = RPZActions::lowerAtom(lowerLayoutTarget);
        QObject::connect(
            lowerAction, &QAction::triggered,
            [=]() {_moveSelectionToLayer(lowerLayoutTarget);}
        );
        layerActions.append(lowerAction);
    
    return layerActions;
}

QList<QAction*> MapLayoutTree::_genVisibilityActions(QList<QTreeWidgetItem*> &selectedItems) {
   
    QList<QAction*> out;

    //helper
    auto _visibilityHelper = [&](bool isHidden) {
        auto selectedIds = this->_selectedAtomIds();
        this->_hints->handleAlterationRequest(VisibilityPayload(selectedIds, isHidden));
    };

    auto showAction = RPZActions::showAtom();
    QObject::connect(
        showAction, &QAction::triggered,
        [=]() {_visibilityHelper(false);}
    );
    out.append(showAction);

    auto hideAction = RPZActions::hideAtom();
    QObject::connect(
        hideAction, &QAction::triggered,
        [=]() {_visibilityHelper(true);}
    );
    out.append(hideAction);

    return out;

}
QList<QAction*> MapLayoutTree::_genAvailabilityActions(QList<QTreeWidgetItem*> &selectedItems) {
    
    QList<QAction*> out;

    //helper
    auto _availabilityHelper = [&](bool isLocked) {
        auto selectedIds = this->_selectedAtomIds();
        this->_hints->handleAlterationRequest(LockingPayload(selectedIds, isLocked));
    };

    auto lockAction = RPZActions::lockAtom();
    QObject::connect(
        lockAction, &QAction::triggered,
        [=]() {_availabilityHelper(true);}
    );
    out.append(lockAction);

    auto unlockAction = RPZActions::unlockAtom();
    QObject::connect(
        unlockAction, &QAction::triggered,
        [=]() {_availabilityHelper(false);}
    );
    out.append(unlockAction);

    return out;

}

void MapLayoutTree::_generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay) {
    
    //if still no item, stop
    if(!itemsToProcess.count()) return;

    //display menu
    QMenu menu;
    menu.addActions(this->_genLayerActions(itemsToProcess));
    menu.addSeparator();
    menu.addActions(this->_genVisibilityActions(itemsToProcess));
    menu.addSeparator();
    menu.addActions(this->_genAvailabilityActions(itemsToProcess));
    menu.addSeparator();

    auto del = RPZActions::remove();
    QObject::connect(
        del, &QAction::triggered,
        [&]() {
            auto selectedIds = this->_selectedAtomIds();
            this->_hints->handleAlterationRequest(RemovedPayload(selectedIds));
        }
    );
    menu.addAction(del);

    menu.exec(whereToDisplay);
}


void MapLayoutTree::keyPressEvent(QKeyEvent * event) {
    
    RPZTree::keyPressEvent(event);

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:

            //make sure there is a selection
            const auto selectedIds = this->_hints->_selectedAtomIds();
            if(!selectedIds.length()) return;

            this->_hints->handleAlterationRequest(RemovedPayload(selectedIds));
            break;
    }

}
