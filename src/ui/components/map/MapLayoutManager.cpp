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
    emit elementsAlterationAsked(this->_extractIdsFromSelection(), MapHint::Alteration::Focused);
}

void MapLayoutManager::_onElementSelectionChanged() {
    
    if(this->_externalInstructionPending || this->_deletionProcessing) return;

    emit elementsAlterationAsked(this->_extractIdsFromSelection(), MapHint::Alteration::Selected);
}

void MapLayoutManager::alterTreeElements(QList<Asset> &elements, const MapHint::Alteration &state) {
   
    this->_externalInstructionPending = true;

    //special handling
    if(state == MapHint::Alteration::Selected) this->clearSelection();
    if(state == MapHint::Alteration::Removed) this->_deletionProcessing = true;

    //iterate through items
    for (auto &e : elements) {

        const auto key = e.id();

        switch(state) {

            case MapHint::Alteration::Removed:
                if(this->_treeItemsById.contains(key)) {
                    delete this->_treeItemsById.take(key);
                }
                break;

            case MapHint::Alteration::Selected:
                if(this->_treeItemsById.contains(key)) {
                    this->_treeItemsById[key]->setSelected(true);
                }
                break;

            case MapHint::Alteration::Added:
                auto item = this->_createTreeItem(e);
                this->_treeItemsById.insert(key, item);
                break;
        }
    }

   
    this->_deletionProcessing = false;
    this->_externalInstructionPending = false;
}

QTreeWidgetItem* MapLayoutManager::_createTreeItem(const Asset &asset) {
    
    auto item = new QTreeWidgetItem(this);
    item->setText(0, asset.descriptor());
    item->setData(0, Qt::UserRole, asset.id());

    const auto type = asset.type();
    switch(type) {
        case AssetBase::Type::Drawing:
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
            const auto selectedIds = this->_extractIdsFromSelection();
            if(!selectedIds.length()) return;

            emit elementsAlterationAsked(selectedIds, MapHint::Alteration::Removed);
            break;
    }

}

QList<QUuid> MapLayoutManager::_extractIdsFromSelection() const {
    
    QList<QUuid> idList;
    
    for(auto &i : this->selectedItems()) {
        const auto innerData = i->data(0,Qt::UserRole).toUuid();
        idList.append(innerData);
    }

    return idList;
}
