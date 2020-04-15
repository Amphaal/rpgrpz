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

#include "src/shared/renderer/graphics/_generic/MapViewGraphicsPathItem.hpp"
#include "src/shared/models/RPZUser.h"
#include <QTimer>

class QuickDrawItem : public MapViewGraphicsPathItem {
    public:
        using Id = SnowFlake::Id;

        QuickDrawItem(const QuickDrawItem::Id &id, const QColor &color, const QPainterPath &firstBits = QPainterPath()) : QuickDrawItem() {
            QPen pen;
            pen.setColor(color);
            this->setPen(pen);
            this->_id = id;
            this->addPathBits(firstBits);
        }
        QuickDrawItem(const RPZUser &emiter) : QuickDrawItem(SnowFlake::get()->nextId(), emiter.color()) {}

        QuickDrawItem::Id id() const {
            return this->_id;
        }

        void moveLine(const QPointF &point) {
            
            this->_path.enqueue(point);

            auto p = this->path();
            p.lineTo(point);
            this->setPath(p);

        }

        void addPathBits(const QPainterPath &bits) {
            //TODO
        }
    
    private:
        QQueue<QPointF> _path;
        QuickDrawItem::Id _id;
        QTimer _tm;

        QuickDrawItem() : MapViewGraphicsPathItem(QPainterPath(), QPen(), QBrush()) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);

            this->_tm.setInterval(2000);
            QObject::connect(
                &this->_tm, &QTimer::timeout,
                this, &QuickDrawItem::_startChomp
            );
            this->_tm.start();

            //TODO add insert animation for remote

        }

        void _startChomp() {
            
            //60 fps
            this->_tm.setInterval(17);

            //if no more left, delete itself
            auto howManyLeft = this->_path.count();
            if(!howManyLeft) {
                this->_tm.stop();
                return this->deleteLater();
            }

            //define erasing speed
            auto decrease = (int)(howManyLeft * 0.005);
            if(decrease < 5) decrease = 5;
            if(howManyLeft < decrease) {
                decrease = howManyLeft;
            }

            //erase from queue
            QPointF moveTo;
            while(decrease) {
                moveTo = this->_path.dequeue();
                decrease--;
            }

            //move start point to latest erased point
            QPainterPath path;
            path.moveTo(moveTo);

            //fill QPainterPath
            for(auto &p : this->_path) {
                path.lineTo(p);
            }

            //determine angle
            QLineF line(moveTo, path.elementAt(0));
            line = QLineF::fromPolar(10, line.angle());
            line.translate(moveTo);

            QRadialGradient grad;
            grad.setColorAt(0, Qt::transparent);
            grad.setColorAt(1, Qt::black);    
            grad.setCenter(line.p1());
            grad.setFocalPoint(line.p2());
            grad.setRadius(120);
   
            QPen p;
            p.setBrush(grad);
            this->setPen(p);

            //refresh path
            this->setPath(path);

        }


};