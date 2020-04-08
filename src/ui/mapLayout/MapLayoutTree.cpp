// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#include "MapLayoutTree.h"

MapLayoutTree::MapLayoutTree(QWidget * parent) : QTreeView(parent) {
    
    auto unselectAction = new QAction;
    unselectAction->setShortcut(QKeySequence::Cancel);
    unselectAction->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
    QObject::connect(
        unselectAction, &QAction::triggered,
        [=]() {
            this->clearSelection();
        }
    );
    this->addAction(unselectAction);

    this->_selectionDebouncer.setInterval(150);
    this->_selectionDebouncer.setSingleShot(true);

    this->_preventSelectionNotification = true;
        this->mlModel = new MapLayoutModel;
        this->setModel(this->mlModel);
    this->_preventSelectionNotification = false;

    this->_menuHandler = new AtomsContextualMenuHandler(this);
    this->_atomActionsHandler = new AtomActionsHandler(this, this);

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
        this->mlModel, &MapLayoutModel::propagateFocus
    );

    //after reset
    QObject::connect(
        this->mlModel, &QAbstractItemModel::modelReset,
        [=]() {
            this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
        }
    );

    QObject::connect(
        this->mlModel, &QAbstractItemModel::rowsInserted,
        [=](const QModelIndex &parent, int first, int last) {
            if(parent.isValid()) {
                this->expand(parent);
                this->sortByColumn(0, Qt::SortOrder::DescendingOrder);
            }
            
        }
    );

    QObject::connect(
        &this->_selectionDebouncer, &QTimer::timeout,
        [=]() {
            this->mlModel->propagateSelection(
                this->selectedIndexes()
            );
        }
    );

}

void MapLayoutTree::_handleAlterationRequest(const AlterationPayload &payload) {

    this->mlModel->payloadTrace(payload);

    this->_preventSelectionNotification = true;

    auto pl = Payloads::autoCast(payload); 

    //handle in database
    this->mlModel->handleAlterationRequest(pl.data());

    //handle in UI
    switch(pl->type()) {
        
        case Payload::Alteration::Selected: {

            auto mPayload = dynamic_cast<const SelectedPayload*>(pl.data());

            QItemSelection newSelection;
            for(const auto &id : mPayload->targetRPZAtomIds()) {
                auto index = this->mlModel->toIndex(id);
                newSelection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
            }

            this->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
            
        }
        break;

        case Payload::Alteration::Focused: {
            
            this->selectionModel()->clear();

            auto mPayload = dynamic_cast<const FocusedPayload*>(pl.data());

            auto itemIndex = this->mlModel->toIndex(mPayload->targetRPZAtomIds().first());
            this->selectionModel()->setCurrentIndex(itemIndex, QItemSelectionModel::ClearAndSelect);

            this->scrollTo(itemIndex, QAbstractItemView::ScrollHint::EnsureVisible);

        }
        break;

        default:
        break;

    }

    this->_preventSelectionNotification = false;

}

void MapLayoutTree::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    QTreeView::selectionChanged(selected, deselected);
    if(this->_preventSelectionNotification) return;
    this->_selectionDebouncer.start();
}

void MapLayoutTree::contextMenuEvent(QContextMenuEvent *event) {

    auto ids = MapLayoutModel::fromIndexes(this->selectedIndexes());

    if(!ids.count()) return;

    //create menu
    this->_menuHandler->invokeMenu(ids, event->globalPos());

}

const QList<RPZAtom::Id> MapLayoutTree::selectedIds() const {
    return MapLayoutModel::fromIndexes(this->selectedIndexes());
}