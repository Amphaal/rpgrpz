#pragma once

#include <QGraphicsItem>
#include <QVariant>

#include <QDebug>

#include "src/shared/models/Payloads.h"

#include "src/shared/models/entities/RPZAtom.hpp"

class MapViewItemsNotified  {
    public:
        virtual void onItemChange(QGraphicsItem* item, AlterationPayload::Alteration alteration) = 0;
};

class MapViewItemsNotifier  {
    public:
        MapViewItemsNotifier(MapViewItemsNotified* targetToNotify, QGraphicsItem* item) : _toNotify(targetToNotify), _item(item) { 
          this->activateNotifications();
        }

        void disableNotifications() {
            
            //default flags
            auto flags = QFlags<QGraphicsItem::GraphicsItemFlag>(
                QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
                QGraphicsItem::GraphicsItemFlag::ItemIsMovable
            );
            this->_item->setFlags(flags);

            this->_mustNotify = false;
        }

        void activateNotifications() {

            //default flags
            auto flags = QFlags<QGraphicsItem::GraphicsItemFlag>(
                QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
                QGraphicsItem::GraphicsItemFlag::ItemIsMovable |
                QGraphicsItem::GraphicsItemFlag::ItemSendsGeometryChanges
            );
            this->_item->setFlags(flags);

            this->_mustNotify = true;
        }
    
    protected:
        void _notifyItemChange(QGraphicsItem::GraphicsItemChange change) {
            if(!this->_item->scene()) return;
            if(!this->_mustNotify) return;
            
            switch(change) {
                case QGraphicsItem::ItemPositionHasChanged:
                    this->_toNotify->onItemChange(this->_item, AlterationPayload::Alteration::Moved);
                    break;
            }
        }

    private:    
        MapViewItemsNotified* _toNotify = nullptr;
        QGraphicsItem* _item = nullptr;
        bool _mustNotify = true;
};