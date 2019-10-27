#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(AtomsStorage* mapMaster, QWidget * parent) : QTreeView(parent) {
    
    this->_model = new MapLayoutModel;
    this->setModel(this->_model);

    this->_menuHandler = new AtomsContextualMenuHandler(mapMaster, this);

    this->setHeaderHidden(true);
    this->setUniformRowHeights(true);
	this->setSortingEnabled(true);

    this->header()->setSortIndicatorShown(false);
    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    this->setItemDelegateForColumn(1, new LockAndVisibilityDelegate);
    
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    this->_handleHintsSignalsAndSlots();

}

void MapLayoutTree::_handleHintsSignalsAndSlots() {

    //on map loading, disable
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
        [=]() {
            this->setEnabled(false);
        }
    );
    
    //handle alteration
    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &MapLayoutTree::_handleAlterationRequest
    );

    //focus
    QObject::connect(
        this, &QTreeView::doubleClicked,
        this->_model, MapLayoutModel::propagateFocus
    );

}

void MapLayoutTree::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

    //clear focus if empty
    auto selectedIndexes = selected.indexes();
    if(!selectedIndexes.count()) this->clearFocus();

    this->_model->propagateSelection(selectedIndexes);
    
}

void MapLayoutTree::_handleAlterationRequest(const AlterationPayload &payload) {
            
    auto pl = Payloads::autoCast(payload); 
    auto type = pl->type();

    this->_model->handleAlterationRequest(pl.data());

    QSignalBlocker b(this);

    switch(type) {
        
        case Payload::Alteration::Selected: {

            auto mPayload = dynamic_cast<const SelectedPayload*>(pl.data());
            
            this->selectionModel()->clearSelection();
            
            for(auto &id : mPayload->targetRPZAtomIds()) {
                auto index = this->_model->toIndex(id);
                this->selectionModel()->select(index, QItemSelectionModel::Select);
            }
            
        }
        break;

        case Payload::Alteration::Focused: {

            auto mPayload = dynamic_cast<const FocusedPayload*>(pl.data());

            this->selectionModel()->clearSelection();

            auto itemIndex = this->_model->toIndex(mPayload->targetRPZAtomIds().first());
            this->selectionModel()->setCurrentIndex(itemIndex, QItemSelectionModel::NoUpdate);

            this->scrollTo(itemIndex, QAbstractItemView::ScrollHint::PositionAtCenter);

        }
        break;

    }


    //in case of disabling from heavy alteration
    this->setEnabled(true); 

    if(type == Payload::Alteration::Reset || type == Payload::Alteration::Added) this->sortByColumn(0, Qt::SortOrder::DescendingOrder);

    this->_resizeSections();
    
}

void MapLayoutTree::_resizeSections() {
    this->header()->resizeSections(QHeaderView::ResizeMode::ResizeToContents);
}

void MapLayoutTree::contextMenuEvent(QContextMenuEvent *event) {

    auto ids = MapLayoutModel::fromIndexes(this->selectedIndexes());

    //create menu
    this->_menuHandler->invokeMenu(ids, event->globalPos());

}

void MapLayoutTree::keyPressEvent(QKeyEvent * event) {
    
    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->_menuHandler->removeSelectedAtoms();
            break;
    }

    QTreeView::keyPressEvent(event);
    
}