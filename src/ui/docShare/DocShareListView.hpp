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

class DocShareListView : public QListWidget, public ConnectivityObserver {
    public:
        DocShareListView(QWidget *parent = nullptr) : QListWidget(parent) {
            this->setAcceptDrops(true);
        }

    protected:
        void connectingToServer() override {
            //TODO
        }
        void connectionClosed(bool hasInitialMapLoaded) override {
            //TODO
        }

    private:
        QMimeDatabase _MIMEDb;

        Qt::DropActions supportedDropActions() const override {
            return (
                Qt::DropAction::MoveAction 
                | Qt::DropAction::CopyAction
            );
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

                auto filename = url.fileName();
                QFileInfo fi(filename);
                auto icon = QFileIconProvider().icon(fi);
                auto playlistItem = new QListWidgetItem(icon, filename);   
                this->addItem(playlistItem);

            }

        }

};