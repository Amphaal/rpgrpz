#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

#include "MapViewItemsNotifier.hpp"

#include <QVariant>
#include <QBrush>
#include <QFont>

class MapViewGraphicsPathItem : public QGraphicsPathItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsPathItem(MapViewItemsNotified* toNotify, const QPainterPath & path, const QPen &pen, const QBrush &brush = QBrush()) : 
        QGraphicsPathItem(path), 
        MapViewItemsNotifier(toNotify, this)  {
            this->setPen(pen);
            if(brush.style() != Qt::BrushStyle::NoBrush) this->setBrush(brush);
        }
            
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsPathItem::itemChange(change, value);
        }
};

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

    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsTextItem::itemChange(change, value);
        }
    
    private:
        void focusInEvent(QFocusEvent  * event) override {
            QGraphicsTextItem::focusInEvent(event);
            MapViewItemsNotifier::_notifyItemChange((int)MapViewCustomItemsEventFlag::TextFocusIn);
        }

        void focusOutEvent(QFocusEvent * event) override {
            QGraphicsTextItem::focusOutEvent(event);
            MapViewItemsNotifier::_notifyItemChange((int)MapViewCustomItemsEventFlag::TextFocusOut);
        }
};

class MapViewGraphicsRectItem : public QGraphicsRectItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsRectItem(MapViewItemsNotified* toNotify, const QRectF & rect, const QPen &pen, const QBrush &brush) :
        QGraphicsRectItem(rect), 
        MapViewItemsNotifier(toNotify, this) {
            this->setBrush(brush);
            this->setPen(pen);
        }
            
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsRectItem::itemChange(change, value);
        }
};

class MapViewGraphicsSvgItem : public QGraphicsSvgItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsSvgItem(MapViewItemsNotified* toNotify, const QString &fileName) : 
        QGraphicsSvgItem(fileName), 
        MapViewItemsNotifier(toNotify, this) {}
    
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsSvgItem::itemChange(change, value);
        }
};

class MapViewGraphicsPixmapItem : public QGraphicsPixmapItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsPixmapItem(MapViewItemsNotified* toNotify, const QString &fileName) : 
        QGraphicsPixmapItem(QPixmap(fileName)), 
        MapViewItemsNotifier(toNotify, this) {}

    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsPixmapItem::itemChange(change, value);
        }
};