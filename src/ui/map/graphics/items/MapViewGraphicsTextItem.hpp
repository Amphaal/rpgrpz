#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "src/ui/map/graphics/MapViewItemsNotifier.h"

#include <QVariant>
#include <QBrush>
#include <QFont>

class MapViewGraphicsTextItem : public QGraphicsTextItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsTextItem(MapViewItemsNotified* toNotify, const QString &text, int textSize) : 
        QGraphicsTextItem(text), 
        MapViewItemsNotifier(toNotify, this) {
            
            auto font = QFont();
            font.setPointSize(textSize);
            this->setFont(font);
            
            //TODO bug "Unsupported composition mode"
            this->setTextInteractionFlags(Qt::TextEditorInteraction);
        }

    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsTextItem::itemChange(change, value);
        }

        void focusInEvent(QFocusEvent * event) override {
            QGraphicsTextItem::focusInEvent(event);
            MapViewItemsNotifier::_notifyItemChange((int)MapViewCustomItemsEventFlag::TextFocusIn);
        }

        void focusOutEvent(QFocusEvent * event) override {
            QGraphicsTextItem::focusOutEvent(event);
            MapViewItemsNotifier::_notifyItemChange((int)MapViewCustomItemsEventFlag::TextFocusOut);
        }
};