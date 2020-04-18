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

class MapViewGraphicsPathItem : public QObject, public QGraphicsPathItem, public RPZGraphicsItem {

    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QPainterPath path READ path WRITE setPath)

    public:
        MapViewGraphicsPathItem(const QPainterPath &path, const QPen &pen, const QBrush &brush = QBrush()) : QGraphicsPathItem(path),
        _sourceBrush(brush) {
            this->_sourceBrushSize = QSizeF(this->_sourceBrush.texture().size());
            this->setPen(pen);
        }

        QBrush sourceBrush() {
            return this->_sourceBrush;
        }

        QSizeF sourceBrushSize() {
            return this->_sourceBrushSize;
        }

    private:
        QBrush _sourceBrush;
        QSizeF _sourceBrushSize;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            QGraphicsPathItem::paint(painter, &result.options, widget);
        }


};