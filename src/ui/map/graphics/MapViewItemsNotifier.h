#pragma once

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QVariant>
#include <QDebug>

enum class MapViewCustomItemsEventFlag {
    Moved = 6224
};

class MapViewItemsNotified  {
    public:
        virtual void _onGraphicsItemCustomChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) = 0;
};

class MapViewItemsNotifier  {
    
    public:
        ~MapViewItemsNotifier();
        static QFlags<QGraphicsItem::GraphicsItemFlag> defaultFlags();

        MapViewItemsNotifier(MapViewItemsNotified* targetToNotify, QGraphicsItem* item);

        void disableNotifications();
        void activateNotifications();
    
    protected:
        void _notifyItemChange(int change);

    private:    
        MapViewItemsNotified* _toNotify = nullptr;
        QGraphicsItem* _item = nullptr;
        bool _mustNotify = true;

        QGraphicsItem* _debugPoint = nullptr;
        
        void _updateDebugPoint();
};