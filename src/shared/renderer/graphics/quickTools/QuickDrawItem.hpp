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
#include "src/shared/models/RPZQuickDrawBits.hpp"

#include <QTimer>

class QuickDrawItem : public MapViewGraphicsPathItem {
    public:


        QuickDrawItem(const RPZQuickDrawBits::Id &id, const QColor &color) : QuickDrawItem() {
            QPen pen;
            pen.setWidth(5);
            pen.setColor(color);
            this->setPen(pen);
            this->_id = id;
        }
        QuickDrawItem(const RPZUser &emiter) : QuickDrawItem(SnowFlake::get()->nextId(), emiter.color()) {}

        RPZQuickDrawBits::Id id() const {
            return this->_id;
        }

        void moveLine(const QPointF &point) {
            
            this->_path.enqueue(point);
            this->_pathPush.enqueue(point);

            auto p = this->path();
            p.lineTo(point);
            this->setPath(p);

            //reset timing before chomping
            this->_tmChomp.setInterval(2000);
            this->_tmChomp.start();

        }

        QPainterPath dequeuePushPoints() {
            
            auto copy = this->_pathPush;
            this->_pathPush.clear();

            QPainterPath out;
            for(auto &point : copy) {
                out.lineTo(point);
            }

            return out;
            
        }

        void addPathBits(const QPainterPath &bits, bool isLastBit) {
            
            if(this->_registeredForDeletion) return;
            if(isLastBit) this->_registeredForDeletion = true;

            //fill buffer
            for(auto i = 0; i < bits.elementCount(); i++) {
                this->_pathPush.enqueue(bits.elementAt(i));
            }

            //restart timer if stopped
            if(!this->_tmPush.isActive()) {
                this->_tmPush.start();
            }
            
        }

        void registerForDeletion() {
            this->_registeredForDeletion = true;
        }
    
    private:
        QQueue<QPointF> _pathPush;
        QQueue<QPointF> _path;
        RPZQuickDrawBits::Id _id;
        QTimer _tmChomp;
        QTimer _tmPush;
        bool _registeredForDeletion = false;
        static inline int _defaultTimerInterval = 17; //60 fps

        QuickDrawItem() : MapViewGraphicsPathItem(QPainterPath(), QPen(), QBrush()) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);

            QObject::connect(
                &this->_tmChomp, &QTimer::timeout,
                this, &QuickDrawItem::_chomp
            );

            //fixed timing
            this->_tmPush.setInterval(_defaultTimerInterval);
            QObject::connect(
                &this->_tmPush, &QTimer::timeout,
                this, &QuickDrawItem::_push
            );

        }

        void _push() {
            
            //check remainings
            auto howManyLeft = this->_pathPush.count();
            if(!howManyLeft) {
                this->_tmPush.stop();
                return;
            }

            //define erasing speed
            auto decrease = _pointDecreaseRate(howManyLeft);

            auto p = this->path();

            //dequeue from buffer and enqueue
            QPointF moveTo;
            while(decrease) {
                moveTo = this->_pathPush.dequeue();
                this->_path.enqueue(moveTo);
                p.lineTo(moveTo);
                decrease--;
            }

            //update path
            this->setPath(p);

        }

        void _chomp() {
            
            //set default timing
            this->_tmChomp.setInterval(_defaultTimerInterval);

            //if no more left, delete itself
            auto howManyLeft = this->_path.count();
            if(!howManyLeft) {
                
                //if no more left and registered for deletion
                if(this->_registeredForDeletion) {
                    this->_tmChomp.stop();
                    this->deleteLater();
                }

                return; 

            }

            //define erasing speed
            auto decrease = _pointDecreaseRate(howManyLeft);

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
   
            auto p = this->pen();
            p.setBrush(grad);
            this->setPen(p);

            //refresh path
            this->setPath(path);

        }

        int _pointDecreaseRate(int remainingPoints) {
            auto decrease = (int)(remainingPoints * 0.005);
            if(decrease < 5) decrease = 5;
            if(remainingPoints < decrease) {
                decrease = remainingPoints;
            }
            return decrease;
        }




};