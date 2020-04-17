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
#include <QTimer>
#include <QPropertyAnimation>

#include "src/helpers/_appContext.h"

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class PingItem : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_INTERFACES(QGraphicsItem)
    
    private:
        QGraphicsView* _view = nullptr;
        QPointF _sceneEvtPoint;
        QColor _pingColor;
        QTimer _tmAutoFadeout;
        QPropertyAnimation* _animFadeout = nullptr;

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            auto &sceneRect = option->exposedRect;
            auto isInScene = sceneRect.contains(this->_sceneEvtPoint);

            QRectF out({}, QSizeF(60, 60));

            if(isInScene) {
                out.moveCenter(this->_sceneEvtPoint);
                this->_mightStartFadeout();
            }

            else {
                
                //resize
                out.setSize(QSizeF(40, 40));       

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
                    out.moveTopLeft(intersected.topLeft());
                } 
                
                else if (angle > 180 && angle <= 270) {
                    out.moveBottomLeft(intersected.bottomLeft());
                } 
                
                else {
                    out.moveBottomRight(intersected.bottomRight());
                }

            }

            painter->drawRect(out);

        }

        void _mightStartFadeout() {
            if(this->_animFadeout->state() != QAbstractAnimation::State::Running) 
                this->_animFadeout->start();
        }

    public:
        ~PingItem() {
            if(_animFadeout) delete _animFadeout;
        }

        PingItem(const QPointF &scenePosPoint, const QColor &pingColor, QGraphicsView* view) : _view(view) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);            

            this->setZValue(AppContext::WALKER_Z_INDEX);

            this->_sceneEvtPoint = scenePosPoint;
            this->_pingColor = pingColor;

            //start auto fadeout after
            this->_tmAutoFadeout.setInterval(3000);
            this->_tmAutoFadeout.start();

            //define fadeout animation
            this->_animFadeout = new QPropertyAnimation(this, "opacity");
            this->_animFadeout->setEasingCurve(QEasingCurve::Linear);
            this->_animFadeout->setDuration(2000);
            this->_animFadeout->setStartValue(1);
            this->_animFadeout->setEndValue(0);

            QObject::connect(
                &this->_tmAutoFadeout, &QTimer::timeout,
                this, &PingItem::_mightStartFadeout
            );

            QObject::connect(
                this->_animFadeout, &QPropertyAnimation::finished,
                this, &QObject::deleteLater
            );

        }

        QRectF boundingRect() const override {
            return this->_view->mapToScene(this->_view->rect()).boundingRect();
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

};