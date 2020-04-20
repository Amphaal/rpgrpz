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
#include <QTimer>
#include <QPropertyAnimation>

#include "src/helpers/_appContext.h"

#include "src/shared/renderer/graphics/MapViewGraphics.h"

class PingIndicatorItem : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_INTERFACES(QGraphicsItem)

 private:
    QGraphicsView* _view = nullptr;
    QTimer _tmAutoFadeout;
    QPropertyAnimation* _animFade = nullptr;

    QPointF _toWatchScenePos;
    QColor _color;
    bool _hasFadeOut = false;
    bool _hasFadein = false;

    static inline int _msTimeoutAutoFade = 10000;
    static inline int _msFadeDuration = 2000;

    bool _canBeDrawnInMiniMap() const override {
        return false;
    }

    bool _isWatchedDisplayed(const QStyleOptionGraphicsItem *option) {
        auto &sceneRect = option->exposedRect;
        auto isInScene = sceneRect.contains(this->_toWatchScenePos);
        return isInScene;
    }

    void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
        // prevent painting if watched item is displayed
        if (_isWatchedDisplayed(option)) {
            _mightStartFadeout();
            return;
        }

        // start fadein
        if (!_hasFadein && this->_animFade->state() != QAbstractAnimation::State::Running) {
            this->_animFade->start();
            _hasFadein = true;
        }

        painter->save();

            painter->setBrush(this->_color);
            painter->setPen(Qt::NoPen);

            this->_paintPingIndicator(painter, option);

        painter->restore();
    }

    void _paintPingIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option) {
        // find border point to draw arrow
        QLineF watchLine(this->_toWatchScenePos, option->exposedRect.center());  // find watch line

        // find intersect point
        auto intersectPoint = _getIntersectionPoint(watchLine, option->exposedRect);
        if (intersectPoint.isNull()) return;

        watchLine.setP1(intersectPoint);  // update watchline to intersect point

        // draw indicator
        auto indicatorVector = watchLine;
        indicatorVector.setLength(35);
        QRectF indicatorRect({}, QSizeF(40, 40));
        indicatorRect.moveCenter(indicatorVector.p2());
        for (const auto &path : PingItem::pingPaths(indicatorRect)) {
            painter->drawPath(path);
        }

        // define arrow vector
        auto arrowVector = watchLine;
        arrowVector.setLength(12);
        arrowVector = QLineF(arrowVector.p2(), arrowVector.p1());

        // draw arrow
        QPainterPath arrow;
        arrow.moveTo(arrowVector.p2());  // start from the tip
        arrow.lineTo(arrowVector.normalVector().p2());  // normal angle
        arrow.lineTo(QLineF(arrowVector.p2(), arrowVector.normalVector().p2()).normalVector().p2());  // anti-normal angle
        painter->drawPath(arrow);
    }

    QPointF _getIntersectionPoint(const QLineF &watchLine, const QRectF &exposedRect) const {
        QPointF out;
        QList<QLineF> borderLines;
        borderLines += { exposedRect.topRight(), exposedRect.bottomRight() };
        borderLines += { exposedRect.bottomRight(), exposedRect.bottomLeft() };
        borderLines += { exposedRect.bottomLeft(), exposedRect.topLeft() };
        borderLines += { exposedRect.topLeft(), exposedRect.topRight() };

        for (const auto &borderLine : borderLines) {
            auto intersectType = watchLine.intersects(borderLine, &out);
            if (intersectType == QLineF::IntersectionType::BoundedIntersection) return out;
        }

        return {};
    }

    void _mightStartFadeout() {
        if (!_hasFadeOut) {
            this->_animFade->setEasingCurve(QEasingCurve::Linear);
            this->_animFade->setDuration(_msFadeDuration);
            this->_animFade->setStartValue(this->opacity());
            this->_animFade->setEndValue(0);
            this->_animFade->start();
            _hasFadeOut = true;
        }
    }

 public:
    ~PingIndicatorItem() {
        if (_animFade) delete _animFade;
    }

    PingIndicatorItem(const PingItem* toWatch, QGraphicsView* view) : _view(view) {
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);

        this->setZValue(AppContext::WALKER_Z_INDEX);

        this->_toWatchScenePos = toWatch->pos();
        this->_color = toWatch->color();

        // start auto fadeout after
        this->_tmAutoFadeout.setInterval(_msTimeoutAutoFade);
        this->_tmAutoFadeout.start();

        // define fadeout animation
        this->_animFade = new QPropertyAnimation(this, "opacity");
        this->_animFade->setEasingCurve(QEasingCurve::InQuad);
        this->_animFade->setDuration(_msFadeDuration);
        this->_animFade->setStartValue(0);
        this->_animFade->setEndValue(1);

        QObject::connect(
            &this->_tmAutoFadeout, &QTimer::timeout,
            this, &PingIndicatorItem::_mightStartFadeout
        );

        QObject::connect(
            this->_animFade, &QPropertyAnimation::finished,
            [=]() {
                if (_hasFadeOut) this->deleteLater();
        });
    }

    // always display in view
    QRectF boundingRect() const override {
        return this->_view->mapToScene(this->_view->rect()).boundingRect();
    }

 protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        auto result = this->conditionnalPaint(this, painter, option, widget);
        if (!result.mustContinue) return;
        this->_paint(painter, &result.options, widget);
    }
};
