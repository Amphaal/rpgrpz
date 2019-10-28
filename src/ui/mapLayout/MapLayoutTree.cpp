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
    this->header()->setDefaultSectionSize(1);
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
    this->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

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
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessed,
        [=]() {
            this->setEnabled(true);
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
        this->_model, &MapLayoutModel::propagateFocus
    );

    //after reset
    QObject::connect(
        this->_model, &QAbstractItemModel::modelReset,
        [=]() {
            this->expandAll();
        }
    );

}

void MapLayoutTree::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

    //TODO fix double trigger on focus out / focus in select (comes from selection model clearing...)

    QTreeView::selectionChanged(selected, deselected);

    //clear focus if empty
    auto selectedIndexes = selected.indexes();

    this->_model->propagateSelection(selectedIndexes);
    
}

void MapLayoutTree::_handleAlterationRequest(const AlterationPayload &payload) {

    QSignalBlocker b1(this);
    QSignalBlocker b2(this->selectionModel());
     
    auto pl = Payloads::autoCast(payload); 
    auto type = pl->type();

    //handle in database
    this->_model->handleAlterationRequest(pl.data());
    
    
    //handle in UI
    switch(type) {
        
        case Payload::Alteration::Selected: {

            auto mPayload = dynamic_cast<const SelectedPayload*>(pl.data());

            this->selectionModel()->clear();

            QItemSelection newSelection;
            for(auto &id : mPayload->targetRPZAtomIds()) {
                auto index = this->_model->toIndex(id);
                newSelection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
            }

            this->selectionModel()->select(newSelection, QItemSelectionModel::Select);
            
        }
        break;

        case Payload::Alteration::Focused: {
            
            this->selectionModel()->clear();

            auto mPayload = dynamic_cast<const FocusedPayload*>(pl.data());

            auto itemIndex = this->_model->toIndex(mPayload->targetRPZAtomIds().first());
            this->selectionModel()->setCurrentIndex(itemIndex, QItemSelectionModel::ClearAndSelect);

            this->scrollTo(itemIndex, QAbstractItemView::ScrollHint::EnsureVisible);

        }
        break;

        case Payload::Alteration::Reset:
        case Payload::Alteration::Added: {
            this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
        }

    }

}

void MapLayoutTree::contextMenuEvent(QContextMenuEvent *event) {

    auto ids = MapLayoutModel::fromIndexes(this->selectedIndexes());

    //create menu
    this->_menuHandler->invokeMenu(ids, event->globalPos());

}

void MapLayoutTree::keyPressEvent(QKeyEvent * event) {
    
    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete: {
            this->_menuHandler->removeSelectedAtoms(this->_selectedIds());
        }
        break;
    }

    QTreeView::keyPressEvent(event);
    
}

const QVector<RPZAtomId> MapLayoutTree::_selectedIds() const {
    return MapLayoutModel::fromIndexes(this->selectedIndexes());
}