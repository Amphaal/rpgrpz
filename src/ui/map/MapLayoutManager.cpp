#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QWidget * parent) : RPZTree(parent) {
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
    emit elementsAlterationAsked(RPZAsset::Alteration::Focused, this->_extractIdsFromSelection());
}

void MapLayoutManager::_onElementSelectionChanged() {
    
    if(this->_externalInstructionPending || this->_deletionProcessing) return;

    emit elementsAlterationAsked(RPZAsset::Alteration::Selected, this->_extractIdsFromSelection());
}

void MapLayoutManager::alterTreeElements(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements) {
   
    this->_externalInstructionPending = true;

    //special handling
    if(state == RPZAsset::Alteration::Selected) this->clearSelection();
    if(state == RPZAsset::Alteration::Removed) this->_deletionProcessing = true;
    if(state == RPZAsset::Alteration::Reset) {
        
        //empty
        for(auto item : this->_treeItemsById) {
            delete item;
        }

        this->_treeItemsById.clear();
    }

    //iterate through items
    for (auto &e : elements) {

        const auto key = e.id();

        switch(state) {

            case RPZAsset::Alteration::Removed:
                if(this->_treeItemsById.contains(key)) {
                    delete this->_treeItemsById.take(key);
                }
                break;

            case RPZAsset::Alteration::Selected:
                if(this->_treeItemsById.contains(key)) {
                    this->_treeItemsById[key]->setSelected(true);
                }
                break;

            case RPZAsset::Alteration::Reset:
            case RPZAsset::Alteration::Added:
                auto item = this->_createTreeItem(e);
                this->_treeItemsById.insert(key, item);
                break;
        }
    }

   
    this->_deletionProcessing = false;
    this->_externalInstructionPending = false;
}

QTreeWidgetItem* MapLayoutManager::_createTreeItem(RPZAsset &asset) {
    
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
    
    RPZTree::keyPressEvent(event);

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:

            //make sure there is a selection
            const auto selectedIds = this->_extractIdsFromSelection();
            if(!selectedIds.length()) return;

            emit elementsAlterationAsked(RPZAsset::Alteration::Removed, selectedIds);
            break;
    }

}

QVector<QUuid> MapLayoutManager::_extractIdsFromSelection() const {
    
    QVector<QUuid> idList;
    
    for(auto &i : this->selectedItems()) {
        const auto innerData = i->data(0, Qt::UserRole).toUuid();
        idList.append(innerData);
    }

    return idList;
}
