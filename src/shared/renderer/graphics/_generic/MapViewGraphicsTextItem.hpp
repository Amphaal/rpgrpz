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

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include <QVariant>
#include <QBrush>
#include <QFont>

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class MapViewGraphicsTextItem : public QGraphicsTextItem, public RPZGraphicsItem {
    public:
        MapViewGraphicsTextItem(const QString &text, int textSize) : QGraphicsTextItem(text) {
            auto font = QFont();
            font.setPointSize(textSize);
            this->setFont(font);
        }
    
    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            QGraphicsTextItem::paint(painter, &result.options, widget);
        }
};