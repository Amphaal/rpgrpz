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

#include "ToysTreeView.h"

ToysTreeView::ToysTreeView(QWidget *parent) : QTreeView(parent), AlterationInteractor(Payload::Interactor::Local_ToysTV),
    _MIMEDb(new QMimeDatabase), 
    _model(new ToysTreeViewModel) {     
    
    //generate raw actions
    this->_generateStaticContainerMoveActions();

    //model
    this->setModel(this->_model);
    this->setRootIndex(this->_model->index(0, 0));
    
    //auto expand on insert
    QObject::connect(
        this->_model, &QAbstractItemModel::rowsInserted,
        this, &ToysTreeView::_onRowInsert
    );

    //ui config
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setHeaderHidden(true);
    this->header()->setStretchLastSection(false);
    this->header()->setMinimumSectionSize(15);
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    //drop config
    this->setAcceptDrops(true);
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    this->setDropIndicatorShown(true);

    //selection
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    //handle alteration
    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &ToysTreeView::_handleAlterationRequest
    );

}

void ToysTreeView::contextMenuEvent(QContextMenuEvent *event) {
    
    auto indexesToProcess = this->_selectedElementsIndexes();

    //check selected items (autoselected on right click)
    if(!indexesToProcess.count()) {
    
        //get elem under cursor
        auto index = this->indexAt(event->pos());

        if(index.isValid() && index.flags().testFlag(Qt::ItemFlag::ItemIsEnabled)) {
            indexesToProcess = {index};
        }

    }

    //create menu
    this->_generateMenu(indexesToProcess, event->globalPos());
}

void ToysTreeView::connectingToServer() {

    //when server responded on available
    QObject::connect(
        _rpzClient, &RPZClient::availableAssetsFromServer,
        this, &ToysTreeView::_onAssetsAboutToBeDownloaded
    );

    //import asset
    QObject::connect(
        this->_rpzClient, &RPZClient::receivedAsset,
        this, &ToysTreeView::_onReceivedAsset
    );

}

void ToysTreeView::_onAssetsAboutToBeDownloaded(const QVector<QString> &availableIds) {
    this->_expectedAssetsTBDownloaded = availableIds.count();
    this->_expectedAssetsDownloaded = 0;
}

void ToysTreeView::_onReceivedAsset(RPZAssetImportPackage package) {
    
    //integrate
    this->_model->integrateAsset(package);
    this->_expectedAssetsDownloaded++;

    //update UI for progress
    QMetaObject::invokeMethod(ProgressTracker::get(), "downloadIsProgressing", 
        Q_ARG(ProgressTracker::Kind, ProgressTracker::Kind::Asset), 
        Q_ARG(qint64, this->_expectedAssetsDownloaded)
    );

    //if upload is finished, update UI
    if(this->_expectedAssetsTBDownloaded == this->_expectedAssetsDownloaded) {
        QMetaObject::invokeMethod(ProgressTracker::get(), "downloadHasEnded", 
            Q_ARG(ProgressTracker::Kind, ProgressTracker::Kind::Asset)
        );
    }
    
    //indicate change
    AssetChangedPayload payload(package);
    AlterationHandler::get()->queueAlteration(this->_model, payload);
    
}

QModelIndexList ToysTreeView::_selectedElementsIndexes() {
    QList<QModelIndex> indexes;

    //get list of items
    for(const auto &i : this->selectedIndexes()) {

        //only first column
        if(i.column() > 0) continue;

        //append
        indexes.append(i);
        
    }

    return indexes;
}


///////////////////
// drag and drop //
///////////////////

void ToysTreeView::startDrag(Qt::DropActions supportedActions) {
    
    auto indexes = this->selectedIndexes();
    auto data = model()->mimeData(indexes);
    if (!data) return;

    QDrag drag(this);
    drag.setMimeData(data);

        //customised cursor
        QPixmap pixmap(":/icons/app/app_32.png");
        QPainter paint(&pixmap);
        paint.setPen(QPen("#000000"));
        paint.setBrush(QBrush(Qt::white));
        QRect numberRect(12, 8, 13, 13);
        paint.drawRect(numberRect);
        paint.drawText(numberRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(indexes.count()));
    
    drag.setPixmap(pixmap); 
    
    //exec
    drag.exec(supportedActions, Qt::MoveAction);

}

void ToysTreeView::dragEnterEvent(QDragEnterEvent *event) {

    //if dragged from OS
    auto md = event->mimeData();
    if (md->hasUrls()) {
        
        for(const auto &url : md->urls()) {

            //if is not local file
            if(!url.isLocalFile()) return;
            
            //if mime of url is image
            auto mimeOfFile = this->_MIMEDb->mimeTypeForUrl(url).name();
            if(!mimeOfFile.startsWith("image")) return;
        }

        //if no error, accept
        this->setState(DraggingState); //mandatory for external drop visual features
        event->accept();
    
    } else if(event->source()) {

        //if has a widget attached, OK
        this->setState(DraggingState); //mandatory for external drop visual features
        event->accept();

    }

}

void ToysTreeView::dragMoveEvent(QDragMoveEvent *event) {
    
    QAbstractItemView::dragMoveEvent(event); //mandatory for external drop visual features

    //if has a widget attached, move type drop
    if(event->source()) {
        event->setDropAction(Qt::DropAction::MoveAction);
    }
}

///////////////////////
// END drag and drop //
///////////////////////

/////////////////////
// Contextual menu //
/////////////////////


void ToysTreeView::_generateStaticContainerMoveActions() {

    QList<QAction*> out;

    for(const auto &toCreate : ToysTreeViewItem::movableStaticContainerTypes()) {
        
        auto targetIndex = this->_model->getStaticContainerTypesIndex(toCreate);
        auto icon = this->_model->data(targetIndex, Qt::DecorationRole).value<QIcon>();
        auto name = this->_model->data(targetIndex, Qt::DisplayRole).toString();
        auto action = new QAction(icon, tr("<< Move to \"%1\"").arg(name));
        
        QObject::connect(
            action, &QAction::triggered,
            [=]() {
                this->_model->moveItemsToContainer(
                    targetIndex,
                    this->_selectedElementsIndexes() 
                );
            }
        );

        out += action;

    }

    this->_staticContainerMoveActions = out;

}

void ToysTreeView::_generateMenu(const QList<QModelIndex> &targetIndexes, const QPoint &whereToDisplay) {
    
    //if no items selected, cancel menu creation
    if(!targetIndexes.count()) return;
    
    QMenu menu;

    //if single selection
    if(targetIndexes.count() == 1) {
        
        //sigle selected item
        auto firstItemIndex = targetIndexes.first();
        auto firstItem = ToysTreeViewItem::fromIndex(firstItemIndex);

        //container actions...
        if(firstItem->allowsSubFolderCreation()) {
            
            //folder creation
            auto createFolder = RPZActions::createFolder();
            QObject::connect(
                createFolder, &QAction::triggered,
                [&]() {
                    auto newFolderIndex = this->_model->createFolder(firstItemIndex);
                    this->edit(newFolderIndex);
                }
            );
            menu.addAction(createFolder);
        }
    }

    // check if all selected are deletable type...
    auto areAllDeletable = [targetIndexes]() {
        for(const auto &elemIndex : targetIndexes) {
            auto elem = ToysTreeViewItem::fromIndex(elemIndex);
            if(!elem->isDeletable()) return false;
        }
        return true;
    }();

    //if so...
    if(areAllDeletable) {

        //allow deletion
        auto deleteItem = RPZActions::remove();
        QObject::connect(
            deleteItem, &QAction::triggered,
            [&, targetIndexes]() {
                this->_requestDeletion(targetIndexes);
            }
        );
        menu.addAction(deleteItem);

        //allow move to static folders
        menu.addSeparator();
        menu.addActions(this->_staticContainerMoveActions);

    }

    //display menu
    if(menu.actions().count()) {
        menu.exec(whereToDisplay);
    }
}

/////////////////////////
// END Contextual menu //
/////////////////////////

//auto expand on row insert
void ToysTreeView::_onRowInsert(const QModelIndex &parent, int first, int last) {
    for (; first <= last; ++first) {
        auto index = this->_model->index(first, 0, parent);
        this->expand(index);
    }
}

void ToysTreeView::_requestDeletion(const QModelIndexList &itemsIndexesToDelete) {
    
    //find deletables
    QModelIndexList deletables;
    for(auto const &index : itemsIndexesToDelete) {
        auto item = ToysTreeViewItem::fromIndex(index);
        if(item->isDeletable()) deletables += index;
    }

    //if no deletables, skip
    if(!deletables.count()) return;

    auto title = tr("Delete elements in toy box");
    auto content = tr("Do you confirm deletion of the %1 selected elements ?").arg(deletables.count());

    auto userResponse = QMessageBox::warning(this, title, content, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(userResponse == QMessageBox::Yes) {
        this->_model->removeItems(deletables);
    }
}

void ToysTreeView::keyPressEvent(QKeyEvent * event) {

    //switch
    switch(event->key()) {
        
        case Qt::Key::Key_Delete: {
            auto selectedIndexes = this->_selectedElementsIndexes();
            if(selectedIndexes.count()) {
                this->_requestDeletion(selectedIndexes);
            }
        }
        break;

        case Qt::Key::Key_Escape: {
            this->clearSelection();
        }
        break;

        default:
            break;
    }

    QTreeView::keyPressEvent(event);
}

void ToysTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

    QTreeView::selectionChanged(selected, deselected);

    auto selectedElems = this->_selectedElementsIndexes();
    auto indexesCount = selectedElems.count();
    
    RPZToy defSelect;

    //if only a single selection
    if(indexesCount == 1) {

        auto elem = ToysTreeViewItem::fromIndex(selectedElems.value(0));
        auto elemType = elem->type();

        //check invokability
        if(elem->isInvokable()) {           
            
            defSelect = RPZToy(
                elem->assetCopy(),
                (RPZAtom::Type)elemType
            );
            
        }
        
    }

    //if still the same, dont send
    if(this->_selectedToy == defSelect) return;
    
    //send
    this->_selectedToy = defSelect;
    ToySelectedPayload payload(this->_selectedToy);
    AlterationHandler::get()->queueAlteration(this->_model, payload);
   
}

void ToysTreeView::_handleAlterationRequest(const AlterationPayload &payload) {
    
    this->payloadTrace(payload);

    auto type = payload.type();
    auto listenedForTypes = (type == Payload::Alteration::Selected || type == Payload::Alteration::Reset);
    if(!listenedForTypes) return;

    auto isAssetSelected = !this->_selectedToy.isEmpty();
    if(!isAssetSelected) return;
    
    this->clearSelection();
}
