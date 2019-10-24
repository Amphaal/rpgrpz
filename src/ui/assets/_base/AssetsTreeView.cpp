#include "AssetsTreeView.h"

AssetsTreeView::AssetsTreeView(QWidget *parent) : QTreeView(parent), 
    AlterationActor(AlterationPayload::Source::Local_AtomDB),
    _MIMEDb(new QMimeDatabase), 
    _model(new AssetsTreeViewModel) {     
    
    //generate raw actions
    this->_generateStaticContainerMoveActions();

    //model
    this->setModel(this->_model);

        //helper for root definition
        auto defineRoot = [&]() {
            auto root = this->_model->index(0,0);
            this->setRootIndex(root);
        };
        
        this->expandAll();
        defineRoot();
    
    //auto expand on insert
    QObject::connect(
        this->_model, &QAbstractItemModel::rowsInserted,
        this, &AssetsTreeView::_onRowInsert
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

    //context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(
        this, &QWidget::customContextMenuRequested,
        this, &AssetsTreeView::_renderCustomContextMenu
    );

    //handle alteration
    QObject::connect(
        AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
        this, &AssetsTreeView::_handleAlterationRequest
    );

    //selection
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
}


void AssetsTreeView::connectingToServer() {

    //when server responded on available
    QObject::connect(
        _rpzClient, &RPZClient::availableAssetsFromServer,
        this, &AssetsTreeView::_onAssetsAboutToBeDownloaded
    );

    //import asset
    QObject::connect(
        this->_rpzClient, &RPZClient::receivedAsset,
        this, &AssetsTreeView::_onReceivedAsset
    );

}

void AssetsTreeView::_onAssetsAboutToBeDownloaded(const QVector<QString> &availableIds) {
    this->_expectedAssetsTBDownloaded = availableIds.count();
    this->_expectedAssetsDownloaded = 0;
}

void AssetsTreeView::_onReceivedAsset(RPZAssetImportPackage package) {
    
    //integrate
    auto success = this->assetsModel()->integrateAsset(package);
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
    AlterationHandler::get()->queueAlteration(this, payload);
}

AssetsTreeViewModel* AssetsTreeView::assetsModel() {
    return this->_model;
}

QModelIndexList AssetsTreeView::selectedElementsIndexes() {
    QList<QModelIndex> indexes;

    //get list of items
    for(auto &i : this->selectedIndexes()) {

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

void AssetsTreeView::startDrag(Qt::DropActions supportedActions) {
    
    auto indexes = this->selectedIndexes();
    QMimeData *data = model()->mimeData(indexes);
    if (!data) return;

    QDrag drag(this);

    QPixmap pixmap(":/icons/app/rpgrpz_32.png");
    QPainter paint(&pixmap);
    paint.setPen(QPen("#000000"));
    paint.setBrush(QBrush(Qt::white));
    QRect numberRect(12, 8, 13, 13);
    paint.drawRect(numberRect);
    paint.drawText(numberRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(indexes.count()));
    drag.setPixmap(pixmap);
            
    drag.setMimeData(data);
    drag.exec(supportedActions, Qt::MoveAction);

}

void AssetsTreeView::dragEnterEvent(QDragEnterEvent *event) {

    //if dragged from OS
    auto md = event->mimeData();
    if (md->hasUrls()) {
        
        for(auto &url : md->urls()) {

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

void AssetsTreeView::dragMoveEvent(QDragMoveEvent *event) {
    
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


void AssetsTreeView::_generateStaticContainerMoveActions() {

    QList<QAction*> out;

    for(auto &toCreate : AssetsTreeViewItem::movableStaticContainerTypes()) {
        
        auto targetIndex = this->_model->getStaticContainerTypesIndex(toCreate);
        auto icon = this->_model->data(targetIndex, Qt::DecorationRole).value<QIcon>();
        auto name = this->_model->data(targetIndex, Qt::DisplayRole).toString();
        auto action = new QAction(icon, tr("<< Move to \"%1\"").arg(name));
        
        QObject::connect(
            action, &QAction::triggered,
            [=]() {
                this->assetsModel()->moveItemsToContainer(
                    targetIndex,
                    this->selectedElementsIndexes() 
                );
            }
        );

        out += action;

    }

    this->_staticContainerMoveActions = out;

}

void AssetsTreeView::_renderCustomContextMenu(const QPoint &pos) {
    
    auto indexesToProcess = this->selectedElementsIndexes();

    //check selected items (autoselected on right click)
    if(!indexesToProcess.count()) {
    
        //get elem under cursor
        auto index = this->indexAt(pos);
        if(index.isValid()) {
            indexesToProcess.append(index);
        }

    }

    //create menu
    this->_generateMenu(indexesToProcess, this->viewport()->mapToGlobal(pos));
}

void AssetsTreeView::_generateMenu(const QList<QModelIndex> &targetIndexes, const QPoint &whereToDisplay) {
    
    //if no items selected, cancel menu creation
    if(!targetIndexes.count()) return;
    
    QMenu menu;

    //if single selection
    if(targetIndexes.count() == 1) {
        
        //sigle selected item
        auto firstItemIndex = targetIndexes.first();
        auto firstItem = AssetsTreeViewItem::fromIndex(firstItemIndex);

        //container actions...
        if(firstItem->isContainer() && firstItem->type() != AssetsTreeViewItem::Type::DownloadedContainer) {
            
            //folder creation
            auto createFolder = RPZActions::createFolder();
            QObject::connect(
                createFolder, &QAction::triggered,
                [&, firstItem]() {
                    this->_model->createFolder(firstItemIndex);
                }
            );
            menu.addAction(createFolder);
        }
    }

    // check if all selected are deletable type...
    auto areAllDeletable = [targetIndexes]() {
        for(auto &elemIndex : targetIndexes) {
            auto elem = AssetsTreeViewItem::fromIndex(elemIndex);
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
void AssetsTreeView::_onRowInsert(const QModelIndex &parent, int first, int last) {
    for (; first <= last; ++first) {
        auto index = this->_model->index(first, 0, parent);
        this->expand(index);
    }
}

void AssetsTreeView::_requestDeletion(const QModelIndexList &itemsIndexesToDelete) {

    auto title = tr("Delete elements in toy box");
    auto content = tr("Do you confirm deletion of the %1 selected elements ?").arg(itemsIndexesToDelete.count());

    auto userResponse = QMessageBox::warning(this, title, content, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(userResponse == QMessageBox::Yes) {
        this->_model->removeItems(itemsIndexesToDelete);
    }
}

void AssetsTreeView::keyPressEvent(QKeyEvent * event) {

    //switch
    switch(event->key()) {
        
        case Qt::Key::Key_Delete: {
            auto selectedIndexes = this->selectedElementsIndexes();
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

void AssetsTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    
    auto i = selected.count();
    auto y = deselected.count();

    QTreeView::selectionChanged(selected, deselected);

    auto selectedElems = this->selectedElementsIndexes();
    auto indexesCount = selectedElems.count();
    
    RPZAsset defSelect;

    //if no selection
    if(!indexesCount) {
        this->clearFocus();
    } 
    
    //if only a single selection
    else if(indexesCount == 1) {

        auto elem = AssetsTreeViewItem::fromIndex(selectedElems.value(0));
        auto atomType = elem->atomType();

        if(atomType != AtomType::Undefined) {
            defSelect = elem->asset();
        }

    }

    //if different, send
    if(this->_selectedAsset != defSelect) {
        this->_selectedAsset = defSelect;

        AssetSelectedPayload payload(this->_selectedAsset);
        AlterationHandler::get()->queueAlteration(this, payload);
    }
   
}

void AssetsTreeView::_handleAlterationRequest(const AlterationPayload &payload) {
    
    auto type = payload.type();
    auto listenedForTypes = (type == PayloadAlteration::Selected || type == PayloadAlteration::Reset);
    if(!listenedForTypes) return;

    auto isAssetSelected = !this->_selectedAsset.isEmpty();
    if(!isAssetSelected) return;
    
    this->clearSelection();
}
