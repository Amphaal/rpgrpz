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

#include "src/shared/hints/HintThread.hpp"
#include <QGraphicsView>

#include "src/shared/renderer/graphics/MapViewGraphics.h"

#include "src/helpers/VectorSimplifier.hpp"

class QuickDrawingAssist : public QObject, public ConnectivityObserver {

    Q_OBJECT

    public:
        QuickDrawingAssist(QGraphicsView* view) : _view(view) {}

        void addDrawingPoint(const QPoint &cursorPosInWindow) {
        
            auto scenePos = this->_view->mapToScene(cursorPosInWindow);

            this->_tempDrawing = nullptr;
            this->_beginDrawing(scenePos);

        }

        void updateDrawingPath(const QPoint &evtPoint) {
            
            //if no temp, stop
            if(!this->_tempDrawing) return;
            
            //define destination coordonate
            auto sceneCoord = this->_view->mapToScene(evtPoint);
            auto pathCoord = this->_tempDrawing->mapFromScene(sceneCoord);

            _tempDrawing->moveLine(pathCoord);


        }

        void onMouseRelease() {
            //remove from temporary
            this->_tempDrawing = nullptr;
        }


        void clearDrawings() {
            qDeleteAll(this->_quickDrawings);
            this->_quickDrawings.clear();
            this->_tempDrawing = nullptr;
        }

    protected:
        virtual void connectingToServer() {
            QObject::connect(
                this->_rpzClient, &RPZClient::gameSessionReceived,
                this, &QuickDrawingAssist::_defineSelfUserFromSession
            );
        }
        virtual void connectionClosed(bool hasInitialMapLoaded) {
            this->_currentUser.clear();
        }

    private:
        QGraphicsView* _view = nullptr;
        QuickDrawItem* _tempDrawing = nullptr;
        QHash<QuickDrawItem::Id, QuickDrawItem*> _quickDrawings;
        
        RPZUser _currentUser;
        void _defineSelfUserFromSession(const RPZGameSession &gameSession) {
            Q_UNUSED(gameSession);
            this->_currentUser = this->_rpzClient->identity();
        }

        void _beginDrawing(const QPointF &scenePos) {

            //create item 
            this->_tempDrawing = new QuickDrawItem(this->_currentUser);
            auto _t_Id = this->_tempDrawing->id();

            //remove from hash once destroyed
            QObject::connect(
                this->_tempDrawing, &QObject::destroyed,
                [=](){
                    this->_quickDrawings.remove(_t_Id);
                }
            );

            this->_quickDrawings.insert(this->_tempDrawing->id(), this->_tempDrawing);
                
            //define pos
            this->_tempDrawing->setPos(scenePos);

            //add to scene
            this->_view->scene()->addItem(this->_tempDrawing);

        }

};