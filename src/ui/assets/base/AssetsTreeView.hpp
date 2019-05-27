#pragma once

#include <QMimeDatabase>
#include <QTreeView>
#include <QHeaderView>
#include <QMessageBox>

#include <QMenu>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include "AssetsTreeViewModel.hpp"

class AssetsTreeView : public QTreeView {

    Q_OBJECT

    public:
        AssetsTreeView(QWidget *parent = nullptr) : 
            QTreeView(parent), 
            _MIMEDb(new QMimeDatabase), 
            _model(new AssetsTreeViewModel) 
        { 
            //model
            this->setModel(this->_model);

                //helper for root definition
                auto defineRoot = [&]() {
                    this->setRootIndex(this->_model->index(0,0));
                    this->expandAll();
                };
                defineRoot();

                //redefine root on reset
                QObject::connect(
                    this->_model, &QAbstractItemModel::modelReset,
                    defineRoot
                );
            
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
            this->setDropIndicatorShown(true);
            this->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
            this->setSelectionMode(QAbstractItemView::ExtendedSelection);

            //context menu
            this->setContextMenuPolicy(Qt::CustomContextMenu);
            QObject::connect(
                this, &QWidget::customContextMenuRequested,
                this, &AssetsTreeView::_renderCustomContextMenu
            );
        }

        AssetsTreeViewModel* assetsModel() {
            return this->_model;
        }

        QModelIndexList selectedElementsIndexes() {
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
    
    signals:
        void requestAssetPreview(QString assetLocation);
        void requestPreviewReset();

    private:

        ///////////////////
        // drag and drop //
        ///////////////////

        QMimeDatabase* _MIMEDb = nullptr;
        AssetsTreeViewModel* _model = nullptr;

        void dragEnterEvent(QDragEnterEvent *event) override {

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

        void dragMoveEvent(QDragMoveEvent *event) override {
            
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

        void _renderCustomContextMenu(const QPoint &pos) {
            
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

        void _generateMenu(QList<QModelIndex> &itemsIndexes, const QPoint &whereToDisplay) {
            
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
                if(firstItem->isContainer()) {
                    
                    //folder creation
                    auto createFolder = new QAction("Créer un dossier");
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
                auto deleteItem = new QAction("Supprimer");
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
        void _onRowInsert(const QModelIndex &parent, int first, int last) {
            for (; first <= last; ++first) {
                auto index = this->_model->index(first, 0, parent);
                this->expand(index);
            }
        }

        void _requestDeletion(const QModelIndexList &itemsIndexesToDelete) {

            QString title = "Suppression des elements de la boîte à jouets";
            QString content = "Voulez-vous vraiment supprimer les " + QString::number(itemsIndexesToDelete.count()) + " elements selectionnés ?";

            auto userResponse = QMessageBox::warning(this, title, content, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
            if(userResponse == QMessageBox::Yes) {
                this->_model->removeItems(itemsIndexesToDelete);
            }
        }

        void keyPressEvent(QKeyEvent * event) override {
            
            auto key = (Qt::Key)event->key();

            //switch
            switch(key) {
                case Qt::Key::Key_Delete:
                    auto selectedIndexes = this->selectedElementsIndexes();
                    if(selectedIndexes.count()) {
                        this->_requestDeletion(selectedIndexes);
                    }
                    break;
            }

            return QTreeView::keyPressEvent(event);
        }
        
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override {
            QTreeView::selectionChanged(selected, deselected);

            //if not single selection, skip
            auto selectedIndexes = this->selectedElementsIndexes();
            if(selectedIndexes.count() != 1) {
                emit requestPreviewReset();
                return;
            }
            
            //if selected elem is no item, skip
            auto targetFilePath = this->_model->getFilePathToAsset(selectedIndexes.takeFirst());
            if(targetFilePath.isNull()) {
                emit requestPreviewReset();
                return;
            }

            //request preview
            emit requestAssetPreview(targetFilePath);
        }

        void focusOutEvent(QFocusEvent *event) override {
            this->clearSelection();
        }


};