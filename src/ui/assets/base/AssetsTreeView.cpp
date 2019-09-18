#include "AssetsTreeView.h"

AssetsTreeView::AssetsTreeView(QWidget *parent) : QTreeView(parent), 
    AlterationActor(AlterationPayload::Source::Local_AtomDB),
    _MIMEDb(new QMimeDatabase), 
    _model(new AssetsTreeViewModel) {     
        
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


void AssetsTreeView::onRPZClientConnecting() {

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

void AssetsTreeView::_onReceivedAsset(const RPZAssetImportPackage &package) {
    
    //integrate
    auto toyModel = this->assetsModel()->integrateAsset(package);
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
    auto payload = AssetChangedPayload(toyModel);
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

void AssetsTreeView::_generateMenu(QList<QModelIndex> &itemsIndexes, const QPoint &whereToDisplay) {
    
    //if no items selected, cancel menu creation
    if(!itemsIndexes.count()) return;

    //list of actions to bind to menu
    QList<QAction*> actions;
    
    //if single selection
    if(itemsIndexes.count() == 1) {
        
        //sigle selected item
        auto firstItemIndex = itemsIndexes.first();
        auto firstItem = AssetsDatabaseElement::fromIndex(firstItemIndex);

        //container actions...
        if(firstItem->isContainer() && firstItem->type() != AssetsDatabaseElement::Type::DownloadedContainer) {
            
            //folder creation
            auto createFolder = RPZActions::createFolder();
            QObject::connect(
                createFolder, &QAction::triggered,
                [&, firstItem]() {
                    this->_model->createFolder(firstItemIndex);
                }
            );
            actions.append(createFolder);
        }
    }

    // check if all selected are deletable type...
    auto areAllDeletable = [itemsIndexes]() {
        for(auto &elemIndex : itemsIndexes) {
            auto elem = AssetsDatabaseElement::fromIndex(elemIndex);
            if(!elem->isDeletable()) return false;
        }
        return true;
    }();

    //if so, allow deletion
    if(areAllDeletable) {
        auto deleteItem = RPZActions::remove();
        QObject::connect(
            deleteItem, &QAction::triggered,
            [&, itemsIndexes]() {
                this->_requestDeletion(itemsIndexes);
            }
        );
        actions.append(deleteItem);
    }

    //display menu
    if(actions.count()) {
        QMenu menu;
        menu.addActions(actions);
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

    QString title = "Suppression des elements de la boîte à jouets";
    QString content = "Voulez-vous vraiment supprimer les " + QString::number(itemsIndexesToDelete.count()) + " elements selectionnés ?";

    auto userResponse = QMessageBox::warning(this, title, content, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(userResponse == QMessageBox::Yes) {
        this->_model->removeItems(itemsIndexesToDelete);
    }
}

void AssetsTreeView::keyPressEvent(QKeyEvent * event) {
    
    auto key = (Qt::Key)event->key();

    //switch
    switch(key) {
        
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

    return QTreeView::keyPressEvent(event);
}

void AssetsTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    
    QTreeView::selectionChanged(selected, deselected);

    auto selectedElems = this->selectedElementsIndexes();
    auto indexesCount = selectedElems.count();
    RPZToyMetadata defSelect;

    //if no selection
    if(!indexesCount) {
        this->clearFocus();
    } 
    
    //if only a single selection
    else if(indexesCount == 1) {

        auto elem = AssetsDatabaseElement::fromIndex(selectedElems[0]);
        auto atomType = elem->atomType();

        if(atomType != AtomType::Undefined) {
            defSelect = elem->toyMetadata();
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
    if(type == PA_Selected || type == PA_Reset) this->clearSelection();
}
