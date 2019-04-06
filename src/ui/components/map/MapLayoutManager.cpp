#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QWidget * parent) : QTreeWidget(parent) {
    this->setHeaderHidden(true);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    QObject::connect(
        this, &QTreeWidget::itemSelectionChanged,
        this, &MapLayoutManager::_onElementSelectionChanged
    );

    QObject::connect(
        this, &QTreeWidget::itemDoubleClicked,
        this, &MapLayoutManager::_onElementDoubleClicked
    );
}

void MapLayoutManager::_onElementDoubleClicked(QTreeWidgetItem * item, int column) {
    emit elementsAlterationAsked(this->_extractIdsFromSelection(), MapView::MapElementEvtState::Focused);
}

void MapLayoutManager::_onElementSelectionChanged() {
    
    if(this->_externalInstructionPending) return;

    emit elementsAlterationAsked(this->_extractIdsFromSelection(), MapView::MapElementEvtState::Selected);
}

void MapLayoutManager::alterTreeElements(QList<Asset> elements, MapView::MapElementEvtState state) {
   
    this->_externalInstructionPending = true;

    //special handling
    if(state == MapView::MapElementEvtState::Selected) this->clearSelection();

    //iterate through items
    for (auto e : elements) {

        auto key = e.id();

        switch(state) {

            case MapView::MapElementEvtState::Removed:
                if(this->_treeItemsById.contains(key)) {
                    delete this->_treeItemsById.take(key);
                }
                break;

            case MapView::MapElementEvtState::Selected:
                if(this->_treeItemsById.contains(key)) {
                    this->_treeItemsById[key]->setSelected(true);
                }
                break;

            case MapView::MapElementEvtState::Added:
                auto item = this->_createTreeItem(e);
                this->_treeItemsById.insert(key, item);
                break;
        }
    }

   

    this->_externalInstructionPending = false;
}

QTreeWidgetItem* MapLayoutManager::_createTreeItem(Asset asset) {
    
    auto item = new QTreeWidgetItem(this);
    item->setText(0, asset.descriptor());
    item->setData(0, Qt::UserRole, asset.id());

    auto type = asset.type();
    switch(type) {
        case AssetType::Type::Drawing:
            item->setIcon(0, QIcon(":/icons/app/manager/drawing.png"));
            break;
    }

    this->addTopLevelItem(item);
    return item;
}

void MapLayoutManager::keyPressEvent(QKeyEvent * event) {
    
    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:

            //make sure there is a selection
            auto selectedIds = this->_extractIdsFromSelection();
            if(!selectedIds.length()) return;

            emit elementsAlterationAsked(selectedIds, MapView::MapElementEvtState::Removed);
            break;
    }

}

QList<QUuid> MapLayoutManager::_extractIdsFromSelection() {
    
    QList<QUuid> idList;
    
    for(auto i : this->selectedItems()) {
        auto innerData = i->data(0,Qt::UserRole).toUuid();
        idList.append(innerData);
    }

    return idList;
}
