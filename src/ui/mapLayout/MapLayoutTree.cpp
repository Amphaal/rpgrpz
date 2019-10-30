#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(AtomsStorage* mapMaster, QWidget * parent) : QTreeView(parent) {
    
    this->_model = new MapLayoutModel;
    this->setModel(this->_model);

    this->_menuHandler = new AtomsContextualMenuHandler(mapMaster, this);
    this->_atomActionsHandler = new AtomActionsHandler(mapMaster, this, this);

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
            this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
        }
    );

    QObject::connect(
        this->_model, &QAbstractItemModel::rowsInserted,
        [=](const QModelIndex &parent, int first, int last) {
            if(parent.isValid()) {
                this->expand(parent);
                this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
            }
            
        }
    );

}

void MapLayoutTree::_handleAlterationRequest(const AlterationPayload &payload) {
    
    if(payload.source() == Payload::Source::Local_MapLayout) return;

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

        default:
        break;

    }

}

void MapLayoutTree::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    
    QTreeView::selectionChanged(selected, deselected);

    this->_model->propagateSelection(selected.indexes());

}

void MapLayoutTree::contextMenuEvent(QContextMenuEvent *event) {

    auto ids = MapLayoutModel::fromIndexes(this->selectedIndexes());

    //create menu
    this->_menuHandler->invokeMenu(ids, event->globalPos());

}

const QVector<RPZAtom::Id> MapLayoutTree::selectedIds() const {
    return MapLayoutModel::fromIndexes(this->selectedIndexes());
}