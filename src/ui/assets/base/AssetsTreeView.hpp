#pragma once

#include <QMimeDatabase>
#include <QTreeView>
#include <QHeaderView>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include "AssetsTreeViewModel.hpp"

class AssetsTreeView : public QTreeView {
    public:
        AssetsTreeView(QWidget *parent = nullptr) : QTreeView(parent), _MIMEDb(new QMimeDatabase) {
            
            //model
            auto model = new AssetsTreeViewModel;
            this->setModel(model);
            this->setRootIndex(model->index(0,0));

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
        }
    
    private:
        //drag and drop
        QMimeDatabase* _MIMEDb = nullptr;

        void dragEnterEvent(QDragEnterEvent *event) override {
            QAbstractItemView::dragEnterEvent(event);

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
                event->acceptProposedAction();
            
            } else if(event->source()) {

                //if has a widget attached, OK
                event->acceptProposedAction();

            }
        }
        
        void dragMoveEvent(QDragMoveEvent * event) override {
            QAbstractItemView::dragMoveEvent(event);

            event->acceptProposedAction();
            event->accept();
        }

        void dropEvent(QDropEvent *event) override {
            QAbstractItemView::dropEvent(event);
            
            event->acceptProposedAction();
            event->accept();
        }
};