#pragma once

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
    #include <QTextDocument>

#include "MapViewItemsNotifier.hpp"

#include <QVariant>

class MapViewGraphicsPathItem : public QGraphicsPathItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsPathItem(MapViewItemsNotified* toNotify, const QPainterPath & path, const QPen &pen, QGraphicsItem * parent = 0) : 
        QGraphicsPathItem(path, parent), 
        MapViewItemsNotifier(toNotify, this) {
            this->setPen(pen);
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
            
            this->setTextInteractionFlags(Qt::TextEditorInteraction);
        }

    QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsTextItem::itemChange(change, value);
        }
    
    private:
        void focusInEvent(QFocusEvent  * event) override {
            QGraphicsTextItem::focusInEvent(event);
            MapViewItemsNotifier::_notifyItemChange(MapViewCustomItemsEventFlag::TextFocusIn);
        }

        void focusOutEvent(QFocusEvent * event) override {
            QGraphicsTextItem::focusOutEvent(event);
            MapViewItemsNotifier::_notifyItemChange(MapViewCustomItemsEventFlag::TextFocusOut);
        }
};

class MapViewGraphicsRectItem : public QGraphicsRectItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsRectItem(MapViewItemsNotified* toNotify, const QRectF & rect, const QPen &pen, const QBrush &brush, QGraphicsItem * parent = 0) :
        QGraphicsRectItem(rect, parent), 
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
        MapViewGraphicsSvgItem(MapViewItemsNotified* toNotify, const QString &fileName, QGraphicsItem *parent = nullptr) : 
        QGraphicsSvgItem(fileName, parent), 
        MapViewItemsNotifier(toNotify, this) {}
    
    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsSvgItem::itemChange(change, value);
        }
};

class MapViewGraphicsPixmapItem : public QGraphicsPixmapItem, public MapViewItemsNotifier {
    public:
        MapViewGraphicsPixmapItem(MapViewItemsNotified* toNotify, const QString &fileName, QGraphicsItem * parent = 0) : 
        QGraphicsPixmapItem(QPixmap(fileName), parent), 
        MapViewItemsNotifier(toNotify, this) {}

    private:
        QVariant itemChange(GraphicsItemChange change, const QVariant & value) override {
            MapViewItemsNotifier::_notifyItemChange(change);
            return QGraphicsPixmapItem::itemChange(change, value);
        }
};