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

class MapViewWalkingHelper : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_INTERFACES(QGraphicsItem)
    
    private:
        RPZMapParameters _mapParams;
        QGraphicsView* _view = nullptr;
        QPointF _destScenePos;

        struct PointPos {
            QPoint viewCursorPos;
            QPointF sceneCursorPos;
            QPointF itemCursorPos;
        };

        const MapViewWalkingHelper::PointPos _generatePointPos() const {
            
            PointPos out;

            out.viewCursorPos = this->_view->mapFromGlobal(QCursor::pos());
            out.sceneCursorPos = this->_view->mapToScene(out.viewCursorPos);
            out.itemCursorPos = this->mapFromScene(out.sceneCursorPos);

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

        void _drawRangeEllipse(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewWalkingHelper::PointPos &pp) const {
            
            painter->save();
                
                QPen pen;
                pen.setWidth(0);
                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing, true);
                
                QLineF line({0,0}, pp.itemCursorPos);
                painter->drawLine(line);

                QBrush brush(Qt::BrushStyle::SolidPattern);
                brush.setColor(AppContext::WALKER_COLOR);
                painter->setBrush(brush);

                painter->setOpacity(.8);
                painter->drawEllipse(option->exposedRect);

            painter->restore();

        }

        const QPointF _drawRangeGrid(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewWalkingHelper::PointPos &pp) const {

            painter->save();

                QPen pen;
                pen.setWidth(1);
                pen.setCosmetic(true);
                pen.setColor(AppContext::WALKER_COLOR);
                pen.setStyle(Qt::DashLine);

                QBrush brush(Qt::BrushStyle::SolidPattern);
                brush.setColor(AppContext::WALKER_COLOR);

                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setOpacity(.8);
                painter->setBrush(brush);
                
                auto alignedToGridItemCursorPos = pp.sceneCursorPos;
                this->_mapParams.alignPointToGridCenter(alignedToGridItemCursorPos);
                alignedToGridItemCursorPos = this->mapFromScene(alignedToGridItemCursorPos);

                //draw dest tile
                auto destTile = QRectF(alignedToGridItemCursorPos, this->_mapParams.tileSizeInPoints());
                auto itemDestPos = destTile.center();
                painter->drawRect(destTile);

                pen.setWidth(5);
                painter->setPen(pen);

                //draw line
                QLineF line({0,0}, itemDestPos);
                painter->drawLine(line);

            painter->restore();

            return this->mapToScene(itemDestPos);

        }

        void _drawLinearRangeTextIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewWalkingHelper::PointPos &pp) {
            
            painter->save();

                QLineF line({0,0}, pp.itemCursorPos);
                painter->setTransform(QTransform());

                QFont font;
                font.setPointSize(15);
                painter->setFont(font);
                
                QTextOption aa;
                aa.setWrapMode(QTextOption::NoWrap);

                auto meters = this->_mapParams.distanceIntoIngameMeters(line.length());
                auto text = StringHelper::fromMeters(meters);
                auto textRect = painter->boundingRect(QRectF(), text, aa);
                textRect = this->_adjustText(pp.itemCursorPos, pp.viewCursorPos, textRect);

                painter->drawText(textRect, text, aa);

            painter->restore();

        }

        void _drawGridRangeTextIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewWalkingHelper::PointPos &pp) {
            
            painter->save();

                painter->setTransform(QTransform());

                QFont font;
                font.setPointSize(15);
                painter->setFont(font);
                
                QTextOption aa;
                aa.setWrapMode(QTextOption::NoWrap);

                auto tileWidth = this->_mapParams.tileWidthInPoints();
                auto scenePos = this->scenePos();
                auto x = qAbs(qRound((scenePos.x() - this->_destScenePos.x()) / tileWidth));
                auto y = qAbs(qRound((scenePos.y() - this->_destScenePos.y()) / tileWidth));
                
                auto text = QStringLiteral(u"%1x%2").arg(x).arg(y);
                auto textRect = painter->boundingRect(QRectF(), text, aa);
                textRect = this->_adjustText(pp.itemCursorPos, pp.viewCursorPos, textRect);

                painter->setOpacity(1);
                painter->drawText(textRect, text, aa);

            painter->restore();

        }

        const QRectF _ellispeBoundingRect() const {
            
            auto pp = this->_generatePointPos();

            auto line = QLineF({}, pp.itemCursorPos);
            
            auto sizePart = qAbs(line.length() * 2);
            auto size = QSizeF(sizePart, sizePart);

            QRectF out({}, size);
            out.moveCenter({});

            return out;

        }

        const QRectF _gridBoundingRect() const {
            
            auto pp = this->_generatePointPos();

            //correct
            auto correctedPos = pp.sceneCursorPos;
            this->_mapParams.alignPointToGridCenter(correctedPos);
            correctedPos = this->mapFromScene(correctedPos);

            auto destTileRect = QRectF(correctedPos, this->_mapParams.tileSizeInPoints()).normalized();
            auto rangeRect = QRectF({}, correctedPos).normalized();
            return destTileRect.united(rangeRect);

        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            auto pp = this->_generatePointPos();

            switch(this->_mapParams.movementSystem()) {

                case RPZMapParameters::MovementSystem::Linear: {
                    this->_drawRangeEllipse(painter, option, pp); //draw ellipse
                    this->_drawLinearRangeTextIndicator(painter, option, pp); //print range indicator
                    this->_destScenePos = pp.sceneCursorPos;
                }
                break;

                case RPZMapParameters::MovementSystem::Grid: {
                    this->_destScenePos = this->_drawRangeGrid(painter, option, pp); //draw grid
                    this->_drawGridRangeTextIndicator(painter, option, pp); //print range indicator
                }
                break;

            }

        }

    public:
        MapViewWalkingHelper(const RPZMapParameters &params, QGraphicsItem* toWalk, QGraphicsView* view) : QGraphicsItem(toWalk), _view(view) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemStacksBehindParent, true);

            this->_mapParams = params;

        }

        QRectF boundingRect() const override {
            
            switch(this->_mapParams.movementSystem()) {

                case RPZMapParameters::MovementSystem::Linear: {
                    return this->_ellispeBoundingRect();
                }
                break;

                case RPZMapParameters::MovementSystem::Grid: {
                    return this->_gridBoundingRect();
                }
                break;

            }
            
            return QRectF();

        }

        const QPointF destScenePos() const {
            return this->_destScenePos;
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