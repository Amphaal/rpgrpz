#pragma once

#include <QMimeDatabase>
#include <QTreeView>
#include <QHeaderView>

#include <QMenu>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include "AssetsTreeViewModel.hpp"

class AssetsTreeView : public QTreeView {
    public:
        AssetsTreeView(QWidget *parent = nullptr) : 
            QTreeView(parent), 
            _MIMEDb(new QMimeDatabase), 
            _model(new AssetsTreeViewModel) 
        { 
            //model
            this->setModel(this->_model);
            this->setRootIndex(this->_model->index(0,0));

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
    
    private:
        //drag and drop
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

        void _renderCustomContextMenu(const QPoint &pos) {
            
            QList<QModelIndex> indexes;

            //check selected items (autoselected on right click)
            if(this->selectedIndexes().count()) {
                
                //get list of items
                for(auto &i : this->selectedIndexes()) {

                    //only first column
                    if(i.column() > 0) continue;

                    //append
                    indexes.append(i);
                }
            } else {
                
                //get elem under cursor
                auto index = this->indexAt(pos);
                if(index.isValid()) {
                    indexes.append(index);
                }

            }

            //create menu
            this->_generateMenu(indexes, this->viewport()->mapToGlobal(pos));
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

                    //folder deletion
                    auto deleteFolder = new QAction("Supprimer le dossier");
                    actions.append(deleteFolder);

                    //prevent deleting
                    if(firstItem->isStaticContainer()) {
                        deleteFolder->setDisabled(true);
                    }
                }
            }

            // check if all selected are items...
            auto areAllItemsType = [itemsIndexes]() {
                for(auto &elemIndex : itemsIndexes) {
                    auto elem = AssetsDatabaseElement::fromIndex(elemIndex);
                    if(!elem->isItem()) return false;
                }
                return true;
            };
            if(areAllItemsType()) {
                auto deleteItem = new QAction("Supprimer");
                QObject::connect(
                    deleteItem, &QAction::triggered,
                    [&, itemsIndexes]() {
                        this->_model->removeItems(itemsIndexes);
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

};