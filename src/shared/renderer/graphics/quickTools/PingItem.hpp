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

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class PingItem : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_INTERFACES(QGraphicsItem)

 private:
    QColor _pingColor;
    QPropertyAnimation* _animOpacityFadeout = nullptr;
    QPropertyAnimation* _animSizeFadeout = nullptr;

    static inline int _msFadeDuration = 2000;

    void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
        painter->save();

            painter->setBrush(this->_pingColor);
            painter->setPen(Qt::NoPen);

            this->_paintPing(painter, option);

        painter->restore();
    }

    void _paintPing(QPainter *painter, const QStyleOptionGraphicsItem *option) {
        QRectF out = option->exposedRect;

        QPainterPath circles;
        circles.addEllipse(out.marginsRemoved(QMarginsF(5, 5, 5, 5)));
        circles.addEllipse(out.marginsRemoved(QMarginsF(10, 10, 10, 10)));
        circles.addEllipse(out.marginsRemoved(QMarginsF(15, 15, 15, 15)));
        circles.addEllipse(out.marginsRemoved(QMarginsF(20, 20, 20, 20)));
        circles.addEllipse(out.marginsRemoved(QMarginsF(25, 25, 25, 25)));

        QPainterPath cross;
        cross.setFillRule(Qt::FillRule::WindingFill);
        cross.addRect(out.marginsRemoved(QMarginsF(0, 29, 0, 29)));  // left
        cross.addRect(out.marginsRemoved(QMarginsF(29, 0, 29, 0)));  // top

        painter->drawPath(circles);
        painter->drawPath(cross);
    }

 public:
    ~PingItem() {
        if (_animOpacityFadeout) delete _animOpacityFadeout;
        if (_animSizeFadeout) delete _animSizeFadeout;
    }

    PingItem(const QPointF &scenePosPoint, const QColor &pingColor) {
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
        this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);

        this->setZValue(AppContext::WALKER_Z_INDEX);
        this->setPos(scenePosPoint);
        this->_pingColor = pingColor;

        // define fadeout animation
        this->_animOpacityFadeout = new QPropertyAnimation(this, "opacity");
        this->_animOpacityFadeout->setEasingCurve(QEasingCurve::Linear);
        this->_animOpacityFadeout->setDuration(_msFadeDuration);
        this->_animOpacityFadeout->setStartValue(1);
        this->_animOpacityFadeout->setEndValue(0);

        // define scaling animation
        this->_animSizeFadeout = new QPropertyAnimation(this, "scale");
        this->_animSizeFadeout->setEasingCurve(QEasingCurve::Linear);
        this->_animSizeFadeout->setDuration(_msFadeDuration);
        this->_animSizeFadeout->setStartValue(1);
        this->_animSizeFadeout->setEndValue(10);

        this->_animOpacityFadeout->start();
        this->_animSizeFadeout->start();

        QObject::connect(
            this->_animOpacityFadeout, &QPropertyAnimation::finished,
            this, &QObject::deleteLater
        );
    }

    QRectF boundingRect() const override {
        QRectF rect({-30, -30}, QSizeF(60, 60));
        return rect;
    }

 protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        auto result = this->conditionnalPaint(this, painter, option, widget);
        if (!result.mustContinue) return;
        this->_paint(painter, &result.options, widget);
    }
};
