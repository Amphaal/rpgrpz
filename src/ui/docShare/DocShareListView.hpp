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

// Any graphical or audio resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.


#pragma once

#include <QListWidget>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QMimeDatabase>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QFileIconProvider>

#include "src/ui/_others/ConnectivityObserver.h"
#include "src/shared/models/RPZSharedDocument.hpp"

class DocShareListView : public QListWidget, public ConnectivityObserver {
    public:
        DocShareListView(QWidget *parent = nullptr) : QListWidget(parent) {
            this->setAcceptDrops(true);

            QObject::connect(
                this, &QListWidget::itemDoubleClicked,
                this, &DocShareListView::_onItemDoubleClick
            );

        }

    protected:
        void connectingToServer() override {
            //TODO
        }
        void connectionClosed(bool hasInitialMapLoaded) override {
            //TODO
        }

    private:
        QHash<RPZSharedDocument::FileHash, RPZSharedDocument> _store;
        QMimeDatabase _MIMEDb;
        QFileIconProvider _iconProvider;
        static inline int HashRole = 3000; 
        static inline int FilePathRole = 3100; 

        Qt::DropActions supportedDropActions() const override {
            return (
                Qt::DropAction::MoveAction 
                | Qt::DropAction::CopyAction
            );
        }

        void _onItemDoubleClick(QListWidgetItem *item) {
            
            auto pathToFile = item->data(DocShareListView::FilePathRole).toString();
            if(pathToFile.isEmpty()) return;

            AppContext::openFileInOS(pathToFile);

        }

        void _mayStoreAsDocument(const QUrl &fileUrl) {
            
            //try to create a shared document
            RPZSharedDocument doc(fileUrl);
            if(!doc.isSuccess()) return;

            //override file if exists
            auto hash = doc.documentFileHash();
            auto alreadyExists = this->_store.contains(hash);
            this->_store.insert(hash, doc);

            if(alreadyExists) return;

            this->_addItem(doc);

        }

        void _addItem(const RPZSharedDocument &doc) {
            
            auto temporaryFilePath = doc.writeAsTemporaryFile();
            auto filename = doc.documentName();

            //icon
            QFileInfo tempFi(temporaryFilePath);
            auto icon = this->_iconProvider.icon(tempFi);
            
            //create item
            auto playlistItem = new QListWidgetItem(icon, filename);
            playlistItem->setData(DocShareListView::HashRole, doc.documentFileHash());
            playlistItem->setData(DocShareListView::FilePathRole, temporaryFilePath);
            
            //add it
            this->addItem(playlistItem);
            qDebug() << qUtf8Printable(QStringLiteral("File Share : \"%1\" added.").arg(filename));

        }
        
        void dragEnterEvent(QDragEnterEvent *event) override {

            QWidget::dragEnterEvent(event);

            //if dragged from OS
            if (!event->mimeData()->hasUrls()) return;
                
            //create list of handled urls
            event->setDropAction(Qt::DropAction::MoveAction);
            event->acceptProposedAction();
        
        }

        void dragMoveEvent(QDragMoveEvent * event) override {
            event->accept();
        }

        void dropEvent(QDropEvent *event) override {
    
            QListWidget::dropEvent(event);

            //for each link registered
            for(const auto &url : event->mimeData()->urls()) {
                _mayStoreAsDocument(url);
            }

        }

};