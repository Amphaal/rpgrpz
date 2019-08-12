#pragma once

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QVariant>
#include <QDebug>

enum class MapViewCustomItemsEventFlag {
    Moved = 6224
};

class ItemChangedNotified;

class GraphicsItemsChangeNotifier  {
    
    public:
        ~GraphicsItemsChangeNotifier();
        static QFlags<QGraphicsItem::GraphicsItemFlag> defaultFlags();

        GraphicsItemsChangeNotifier(QGraphicsItem* item);

        void disableNotifications();
        void activateNotifications();

        void addNotified(ItemChangedNotified* notified);

        QGraphicsItem* graphicsItem();
    
    protected:
        void _notifyItemChange(int change);

    private:    
        ItemChangedNotified* _toNotify = nullptr;
        QGraphicsItem* _item = nullptr;
        bool _mustNotify = true;
};

class ItemChangedNotified {
    public:
        virtual void onItemChanged(GraphicsItemsChangeNotifier* item, MapViewCustomItemsEventFlag flag) = 0;
};