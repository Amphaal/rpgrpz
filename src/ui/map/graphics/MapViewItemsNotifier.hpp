#pragma once

#include <QGraphicsItem>
#include <QVariant>

#include <QDebug>

#include "src/shared/models/Payloads.h"

#include "src/shared/models/entities/RPZAtom.hpp"

enum MapViewCustomItemsEventFlag {
    Moved = 6224,
    TextFocusOut = 6225,
    TextFocusIn = 6226
};

class MapViewItemsNotified  {
    public:
        virtual void onItemChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) = 0;
};

class MapViewItemsNotifier  {
    
    public:
        static QFlags<QGraphicsItem::GraphicsItemFlag> defaultFlags() {
            return QFlags<QGraphicsItem::GraphicsItemFlag>(
                QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
                QGraphicsItem::GraphicsItemFlag::ItemIsMovable
            );
        }

        MapViewItemsNotifier(MapViewItemsNotified* targetToNotify, QGraphicsItem* item) : _toNotify(targetToNotify), _item(item) { 
            
            this->activateNotifications();
            
            item->setTransformOriginPoint(
                item->boundingRect().center()
            );
        }

        void disableNotifications() {
            this->_item->setFlags(defaultFlags());
            this->_mustNotify = false;
        }

        void activateNotifications() {

            auto flags = defaultFlags();
            flags.setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsGeometryChanges);
            this->_item->setFlags(flags);

            this->_mustNotify = true;
        }
    
    protected:
        void _notifyItemChange(int change) {
            if(!this->_item->scene()) return;
            if(!this->_mustNotify) return;
            
            switch(change) {
                case QGraphicsItem::ItemPositionHasChanged:
                    this->_toNotify->onItemChange(this->_item, MapViewCustomItemsEventFlag::Moved);
                    break;
                case MapViewCustomItemsEventFlag::TextFocusOut:
                case MapViewCustomItemsEventFlag::TextFocusIn:
                    this->_toNotify->onItemChange(this->_item, (MapViewCustomItemsEventFlag)change);
                    break;
            }
        }

    private:    
        MapViewItemsNotified* _toNotify = nullptr;
        QGraphicsItem* _item = nullptr;
        bool _mustNotify = true;
};