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
// different license and copyright still refer to this GPL.

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

#include "src/helpers/StringHelper.hpp"

#include "src/helpers/_appContext.h"

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"
#include "src/shared/models/RPZMapParameters.hpp"
#include "src/shared/renderer/graphics/_specific/MapViewToken.hpp"

class MapViewMeasurementHelper : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_INTERFACES(QGraphicsItem)
    
    private:
        RPZMapParameters _mapParams;
        QGraphicsView* _view = nullptr;
        QPointF _startScenePos;
        
        struct PointPos {
            QPoint viewCursorPos;
            QPointF sceneCursorPos;
            QLineF distanceLine;
        };

        const MapViewMeasurementHelper::PointPos _generateCursorPointPos() const {
            
            PointPos out;
            out.viewCursorPos = this->_view->mapFromGlobal(QCursor::pos());
            out.sceneCursorPos = this->_view->mapToScene(out.viewCursorPos);
            
            if(this->_mapParams.movementSystem() == RPZMapParameters::MovementSystem::Grid) {
                this->_mapParams.alignPointFromStartPoint(out.sceneCursorPos);
            }

            out.distanceLine = QLineF(this->_startScenePos, out.sceneCursorPos);

            return out;
            
        }

        QRectF _adjustText(const QPointF &relativeTo, QPointF dest, QRectF toAdjust) {
            
            QPointF correct;

                if(relativeTo.x() < 0) {
                    correct += {-12, 0};
                } else {
                    correct += {12, 0};
                }

                if(relativeTo.y() < 0) {
                    correct += {0, -12};
                } else {
                    correct += {0, 12};
                }

            dest += correct;

            if(correct.x() < 0 && correct.y() < 0) toAdjust.moveBottomRight(dest);
            else if(correct.x() >= 0 && correct.y() < 0) toAdjust.moveBottomLeft(dest);
            else if(correct.x() < 0 && correct.y() >= 0) toAdjust.moveTopRight(dest);
            else if(correct.x() >= 0 && correct.y() >= 0) toAdjust.moveTopLeft(dest);

            return toAdjust;

        }

        void _defineWalkerGuide(QPainter *painter) const {
            QPen pen;
            pen.setWidth(5);
            pen.setCosmetic(true);
            pen.setColor(AppContext::MEASUREMENT_COLOR);
            pen.setStyle(Qt::DashLine);
            painter->setPen(pen);
                
            QBrush brush(Qt::BrushStyle::SolidPattern);
            brush.setColor(AppContext::MEASUREMENT_COLOR);
            painter->setBrush(brush);

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setOpacity(.8);
        }

        void _drawRangeEllipse(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewMeasurementHelper::PointPos &pp) {
            
            painter->save();
                
                this->_defineWalkerGuide(painter);
                
                //draw line helper
                painter->drawLine(pp.distanceLine);

                //change pen for circle border
                auto pen = painter->pen();
                pen.setWidth(1);
                painter->setPen(pen);

                //draw ellipse helper
                painter->drawEllipse(option->exposedRect);

            painter->restore();

        }

        void _drawRangeGrid(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewMeasurementHelper::PointPos &pp) {

            painter->save();

                this->_defineWalkerGuide(painter);

                //draw line
                painter->drawLine(pp.distanceLine);

            painter->restore();

        }

        void _drawLinearRangeTextIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewMeasurementHelper::PointPos &pp) {
            
            painter->save();

                QLineF line({0,0}, pp.sceneCursorPos);
                painter->setTransform(QTransform());

                QFont font;
                font.setPointSize(15);
                painter->setFont(font);
                
                QTextOption aa;
                aa.setWrapMode(QTextOption::NoWrap);

                auto meters = this->_mapParams.distanceIntoIngameMeters(line.length());
                auto text = StringHelper::fromMeters(meters);
                auto textRect = painter->boundingRect(QRectF(), text, aa);
                textRect = this->_adjustText(pp.sceneCursorPos, pp.viewCursorPos, textRect);

                painter->drawText(textRect, text, aa);

            painter->restore();

        }

        void _drawGridRangeTextIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewMeasurementHelper::PointPos &pp) {
            
            painter->save();

                painter->setTransform(QTransform());

                QFont font;
                font.setPointSize(15);
                painter->setFont(font);
                
                QTextOption aa;
                aa.setWrapMode(QTextOption::NoWrap);

                auto tileWidth = this->_mapParams.tileWidthInPoints();
                auto x = qAbs(qRound((this->_startScenePos.x() - pp.sceneCursorPos.x()) / tileWidth));
                auto y = qAbs(qRound((this->_startScenePos.y() - pp.sceneCursorPos.y()) / tileWidth));
                
                auto text = QStringLiteral(u"%1x%2").arg(x).arg(y);
                auto textRect = painter->boundingRect(QRectF(), text, aa);
                textRect = this->_adjustText(pp.sceneCursorPos, pp.viewCursorPos, textRect);

                painter->setOpacity(1);
                painter->drawText(textRect, text, aa);

            painter->restore();

        }

        const QRectF _ellipseBoundingRect() const {

            auto pp = this->_generateCursorPointPos();
            
            auto sizePart = qAbs(pp.distanceLine.length() * 2);
            auto size = QSizeF(sizePart, sizePart);

            QRectF out(pp.distanceLine.p1(), size);
            out.moveCenter(pp.distanceLine.p1());

            return out;

        }

        const QRectF _gridBoundingRect() const {
            
            auto pp = this->_generateCursorPointPos();
            return QRectF(pp.distanceLine.p1(), pp.distanceLine.p2());

        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            auto pp = this->_generateCursorPointPos();

            switch(this->_mapParams.movementSystem()) {

                case RPZMapParameters::MovementSystem::Linear: {
                    this->_drawRangeEllipse(painter, option, pp); //draw ellipse
                    this->_drawLinearRangeTextIndicator(painter, option, pp); //print range indicator
                }
                break;

                case RPZMapParameters::MovementSystem::Grid: {
                    this->_drawRangeGrid(painter, option, pp); //draw grid
                    this->_drawGridRangeTextIndicator(painter, option, pp); //print range indicator
                }
                break;

            }

        }

    public:
        MapViewMeasurementHelper(const RPZMapParameters &params, const QPoint &evtPosPoint, QGraphicsView* view) : _view(view) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);            

            this->_startScenePos = this->_view->mapToScene(evtPosPoint);
            if(this->_mapParams.movementSystem() == RPZMapParameters::MovementSystem::Grid) {
                this->_mapParams.alignPointFromStartPoint(this->_startScenePos);
            }

            this->_mapParams = params;

            this->setZValue(AppContext::WALKER_Z_INDEX);

        }

        QRectF boundingRect() const override {
            
            auto movementSystem = this->_mapParams.movementSystem();
            if(movementSystem == RPZMapParameters::MovementSystem::Linear) return this->_ellipseBoundingRect();

            return this->_gridBoundingRect();

        }

    protected:
        bool _canBeDrawnInMiniMap() const override { 
            return false; 
        };

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

};