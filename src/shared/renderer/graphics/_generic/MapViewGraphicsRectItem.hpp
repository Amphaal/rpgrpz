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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include <QVariant>
#include <QBrush>
#include <QFont>

#include <QObject>

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class MapViewGraphicsRectItem : public QObject, public QGraphicsRectItem, public RPZGraphicsItem {

    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

    public:
        MapViewGraphicsRectItem(const QRectF &rect, const QPen &pen, const QBrush &brush) : QGraphicsRectItem(rect) {
            this->setBrush(brush);
            this->setPen(pen);
        }

    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            QGraphicsRectItem::paint(painter, &result.options, widget);
        }

};