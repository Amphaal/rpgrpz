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