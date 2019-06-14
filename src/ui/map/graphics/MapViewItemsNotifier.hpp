#pragma once

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QVariant>
#include <QDebug>

enum class MapViewCustomItemsEventFlag {
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
        ~MapViewItemsNotifier() {
            if(this->_debugPoint) delete this->_debugPoint;
        }

        static QFlags<QGraphicsItem::GraphicsItemFlag> defaultFlags() {
            return QFlags<QGraphicsItem::GraphicsItemFlag>(
                QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
                QGraphicsItem::GraphicsItemFlag::ItemIsMovable
            );
        }

        MapViewItemsNotifier(MapViewItemsNotified* targetToNotify, QGraphicsItem* item) : _toNotify(targetToNotify), _item(item) { 
            
            //notifications activated by default
            this->activateNotifications();
            
            //define transform origin point to center
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
            
            // this->_updateDebugPoint();

            switch(change) {
                case QGraphicsItem::ItemPositionHasChanged: {
                    this->_toNotify->onItemChange(this->_item, MapViewCustomItemsEventFlag::Moved);
                }
                break;
                case (int)MapViewCustomItemsEventFlag::TextFocusOut:
                case (int)MapViewCustomItemsEventFlag::TextFocusIn:
                    this->_toNotify->onItemChange(this->_item, (MapViewCustomItemsEventFlag)change);
                    break;
            }
        }

    private:    
        MapViewItemsNotified* _toNotify = nullptr;
        QGraphicsItem* _item = nullptr;
        bool _mustNotify = true;

        QGraphicsItem* _debugPoint = nullptr;
        
        void _updateDebugPoint() {
            
            if(this->_debugPoint) delete this->_debugPoint;

            auto pos = this->_item->pos();
            auto line = QLineF(
                pos,
                pos + QPointF(.01, .01)
            );

            QPen pen;
            pen.setWidth(1);
            pen.setColor("#FF0000");

            this->_debugPoint = this->_item->scene()->addLine(line, pen);

        }
};