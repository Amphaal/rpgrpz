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

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QPointF>
#include <QPainter>
#include <QDebug>
#include <QSizeF>
#include <QFont>
#include <QStyleOptionGraphicsItem>
#include <QTextOption>

#include "src/helpers/_appContext.h"

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class PingItem : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_INTERFACES(QGraphicsItem)
    
    private:
        QGraphicsView* _view = nullptr;
        QPointF _sceneEvtPoint;
        QColor _pingColor;

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            painter->drawRect(option->rect);
        }

    public:
        PingItem(const QPoint &evtPosPoint, const QColor &pingColor, QGraphicsView* view) : _view(view) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);            

            this->setZValue(AppContext::WALKER_Z_INDEX);

            this->_sceneEvtPoint = this->_view->mapToScene(evtPosPoint);
            this->_pingColor = pingColor;

        }

        QRectF boundingRect() const override {
            
            auto sceneRect = this->_view->sceneRect();
            auto isInScene = sceneRect.contains(this->_sceneEvtPoint);

            QRectF out({}, QSizeF(30, 30));

            if(isInScene) {
                out.moveCenter(this->_sceneEvtPoint);
            }

            else {
                
                //resize
                out.setSize(QSizeF(20, 20));       

                //find line and angle
                QLineF line(
                    sceneRect.center(),
                    this->_sceneEvtPoint
                );
                QRectF lineRect(line.p1(), line.p2());

                //intersect to get angle points
                auto intersected = sceneRect.intersected(lineRect);
                
                //move to angle
                auto angle = line.angle();
                if(angle > 0 && angle <= 90) {
                    out.moveTopRight(intersected.topRight());
                } 

                else if (angle > 90 && angle <= 180) {
                    out.moveBottomRight(intersected.bottomRight());
                } 
                
                else if (angle > 180 && angle <= 270) {
                    out.moveBottomLeft(intersected.bottomLeft());
                } 
                
                else {
                    out.moveTopLeft(intersected.topLeft());
                }

            }

            return out;

        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

};