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

class MapViewGraphicsTextItem : public QGraphicsTextItem {
    public:
        MapViewGraphicsTextItem(const QString &text, int textSize) : 
        QGraphicsTextItem(text) {
            auto font = QFont();
            font.setPointSize(textSize);
            this->setFont(font);
        }
};