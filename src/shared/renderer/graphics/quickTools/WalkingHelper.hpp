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

class WalkingHelper : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_INTERFACES(QGraphicsItem)

 private:
    RPZMapParameters _mapParams;
    QGraphicsView* _view = nullptr;
    QList<QGraphicsItem*> _toWalk;
    bool _singleItemToWalk = false;
    QHash<QGraphicsItem*, QPointF> _destinations;

    struct PointPos {
        QPoint viewCursorPos;
        QPointF sceneCursorPos;
        QLineF distanceLine;
    };

    const WalkingHelper::PointPos _generateCursorPointPos() const {
        PointPos out;
        out.viewCursorPos = this->_view->mapFromGlobal(QCursor::pos());
        out.sceneCursorPos = this->_view->mapToScene(out.viewCursorPos);

        auto unitedRect = _getUnitedRect();
        auto alignedSCP = out.sceneCursorPos;

        if (this->_mapParams.movementSystem() == RPZMapParameters::MovementSystem::Grid) {
            this->_mapParams.alignPointToGridTile(alignedSCP, unitedRect.size());
        }

        out.distanceLine = QLineF(unitedRect.center(), alignedSCP);

        return out;
    }

    QRectF _adjustText(const QPointF &relativeTo, QPointF dest, QRectF toAdjust) {
        QPointF correct;

            if (relativeTo.x() < 0) {
                correct += {-12, 0};
            } else {
                correct += {12, 0};
            }

            if (relativeTo.y() < 0) {
                correct += {0, -12};
            } else {
                correct += {0, 12};
            }

        dest += correct;

        if (correct.x() < 0 && correct.y() < 0) toAdjust.moveBottomRight(dest);
        else if (correct.x() >= 0 && correct.y() < 0) toAdjust.moveBottomLeft(dest);
        else if (correct.x() < 0 && correct.y() >= 0) toAdjust.moveTopRight(dest);
        else if (correct.x() >= 0 && correct.y() >= 0) toAdjust.moveTopLeft(dest);

        return toAdjust;
    }

    void _defineWalkerGuide(QPainter *painter) const {
        QPen pen;
        pen.setWidth(5);
        pen.setCosmetic(true);
        pen.setColor(AppContext::WALKER_COLOR);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);

        QBrush brush(Qt::BrushStyle::SolidPattern);
        brush.setColor(AppContext::WALKER_COLOR);
        painter->setBrush(brush);

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setOpacity(.8);
    }

    // calculate offset
    QPointF _getOffset(const WalkingHelper::PointPos &pp, const QGraphicsItem* item) const {
        auto offsetLine = QLineF(pp.distanceLine.p1(), item->scenePos());
        if (offsetLine.p1() == offsetLine.p2()) return QPointF();
        return offsetLine.translated(-offsetLine.p1()).p2();
    }

    void _drawRangeEllipse(QPainter *painter, const QStyleOptionGraphicsItem *option, const WalkingHelper::PointPos &pp) {
        painter->save();

            this->_defineWalkerGuide(painter);

            for (auto item : this->_toWalk) {
                // calculate offset
                auto offset = _getOffset(pp, item);

                // draw line helper
                auto line = pp.distanceLine.translated(offset);
                painter->drawLine(line);

                // draw helper rect
                auto inplaceRect = item->boundingRect();
                inplaceRect.translate(pp.sceneCursorPos + offset);
                painter->drawEllipse(inplaceRect);

                // add destination
                this->_destinations.insert(item, line.p2());
            }

            // if single walk, display indicator
            if (this->_singleItemToWalk) {
                // change pen for circle border
                auto pen = painter->pen();
                pen.setWidth(1);
                painter->setPen(pen);

                // draw ellipse helper
                painter->drawEllipse(option->exposedRect);
            }

        painter->restore();
    }

    void _drawRangeGrid(QPainter *painter, const QStyleOptionGraphicsItem *option, const WalkingHelper::PointPos &pp) {
        painter->save();

            this->_defineWalkerGuide(painter);
            this->_destinations.clear();

            // draw line
            for (auto item : this->_toWalk) {
                // calculate offset
                auto offset = _getOffset(pp, item);
                auto offsetted = pp.distanceLine.p2() + offset;

                painter->save();

                auto able = !isMoveOrInsertPreventedAtPosition(this->_mapParams, item, offsetted);
                if (!able) {
                    // change pen color
                    auto pen = painter->pen();
                    pen.setColor("Red");
                    painter->setPen(pen);
                }

                // draw line helper
                auto line = QLineF(item->scenePos(), offsetted);
                painter->drawLine(line);

                // define rect
                auto inplaceRect = item->boundingRect();
                inplaceRect.moveCenter(offsetted);

                // draw rect or cross
                if (able) {
                    painter->drawRect(inplaceRect);
                } else {
                    painter->drawLine(inplaceRect.topLeft(), inplaceRect.bottomRight());
                    painter->drawLine(inplaceRect.topRight(), inplaceRect.bottomLeft());
                }

                // add destination
                if (able) this->_destinations.insert(item, line.p2());

                painter->restore();
            }

        painter->restore();
    }

    void _drawLinearRangeTextIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option, const WalkingHelper::PointPos &pp) {
        painter->save();

            QLineF line({0, 0}, pp.sceneCursorPos);
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

    void _drawGridRangeTextIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option, const WalkingHelper::PointPos &pp) {
        painter->save();

            painter->setTransform(QTransform());

            QFont font;
            font.setPointSize(15);
            painter->setFont(font);

            QTextOption aa;
            aa.setWrapMode(QTextOption::NoWrap);

            auto tileWidth = this->_mapParams.tileWidthInPoints();
            auto firstItemScenePos = this->_toWalk.first()->scenePos();
            auto x = qAbs(qRound((firstItemScenePos.x() - pp.sceneCursorPos.x()) / tileWidth));
            auto y = qAbs(qRound((firstItemScenePos.y() - pp.sceneCursorPos.y()) / tileWidth));

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

        auto from = _getUnitedRect();
        auto dest = from;
        dest.moveCenter(pp.distanceLine.p2());

        return from.united(dest);
    }

    void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
        auto pp = this->_generateCursorPointPos();

        switch (this->_mapParams.movementSystem()) {
            case RPZMapParameters::MovementSystem::Linear: {
                this->_drawRangeEllipse(painter, option, pp);  // draw ellipse
                if (_singleItemToWalk) this->_drawLinearRangeTextIndicator(painter, option, pp);  // print range indicator
            }
            break;

            case RPZMapParameters::MovementSystem::Grid: {
                this->_drawRangeGrid(painter, option, pp);  // draw grid
                if (_singleItemToWalk) this->_drawGridRangeTextIndicator(painter, option, pp);  // print range indicator
            }
            break;
        }
    }

    QRectF _getUnitedRect() const {
        QRectF out;
        for (auto item : this->_toWalk) {
            out = out.united(item->sceneBoundingRect());
        }
        return out;
    }

 public:
    WalkingHelper(const RPZMapParameters &params, const QList<QGraphicsItem*> &toWalk, QGraphicsView* view) : _toWalk(toWalk), _view(view) {
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);

        this->_mapParams = params;
        this->_singleItemToWalk = toWalk.count() == 1;

        this->setZValue(AppContext::WALKER_Z_INDEX);
    }

    const QHash<QGraphicsItem*, QPointF> destinations() const {
        return this->_destinations;
    }

    QRectF boundingRect() const override {
        auto movementSystem = this->_mapParams.movementSystem();
        if (movementSystem == RPZMapParameters::MovementSystem::Linear && this->_singleItemToWalk) return this->_ellipseBoundingRect();

        return this->_gridBoundingRect();
    }

    static bool isToBeWalked(const WalkingHelper* helper, QGraphicsItem* toCheck) {
        if (!helper) return false;
        return helper->_toWalk.contains(toCheck);
    }

    static bool isMoveOrInsertPreventedAtPosition(const RPZMapParameters &mapParams, const QGraphicsItem* toCheck, const QPointF &toCheckAt = QPointF()) {
        // check if not a grid system
        if (mapParams.movementSystem() != RPZMapParameters::MovementSystem::Grid) return false;

        // check item is not bound to the grid
        if (!dynamic_cast<const RPZGridBound*>(toCheck)) return false;

        // determine destination
        auto atPosRect = toCheck->sceneBoundingRect();
        if (!toCheckAt.isNull()) atPosRect.moveCenter(toCheckAt);

        // check if we move the item onto another grid bound item
        for (const auto colliding : toCheck->scene()->items(atPosRect)) {
            if (colliding == toCheck) continue;
            if (dynamic_cast<const RPZGridBound*>(colliding)) return true;
        }

        return false;
    }

 protected:
    bool _canBeDrawnInMiniMap() const override {
        return false;
    };

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        auto result = this->conditionnalPaint(this, painter, option, widget);
        if (!result.mustContinue) return;
        this->_paint(painter, &result.options, widget);
    }
};
