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

#include "src/ui/_others/ConnectivityObserver.h"

class DocShareListView : public QListWidget, public ConnectivityObserver {
    public:
        DocShareListView(QWidget *parent = nullptr) : QListWidget(parent) {

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

        void dragEnterEvent(QDragEnterEvent *event) override {
            
            //must not be a widget source
            if(event->source()) return;

            //must have urls included
            auto md = event->mimeData();
            if(!md->hasUrls()) return;

            for(const auto &url : md->urls()) {

                //if is not local file
                if(!url.isLocalFile()) return;
                
            }

            //if no error, accept
            this->setState(DraggingState); //mandatory for external drop visual features
            event->accept();

        }

        void startDrag(Qt::DropActions supportedActions) override {

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

};