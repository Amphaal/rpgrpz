#pragma once

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QVariant>
#include <QDebug>

enum class MapViewCustomItemsEventFlag {
    Moved = 6224
};

class ItemChangedNotifier  {
    public:
        virtual void onItemChanged(GraphicsItemsChangeNotifier* item, MapViewCustomItemsEventFlag flag) = 0;
};

class GraphicsItemsChangeNotifier  {
    
    public:
        ~GraphicsItemsChangeNotifier();
        static QFlags<QGraphicsItem::GraphicsItemFlag> defaultFlags();

        GraphicsItemsChangeNotifier(QGraphicsItem* item);

        void disableNotifications();
        void activateNotifications();

        void addNotified(ItemChangedNotifier* notified);

        QGraphicsItem* graphicsItem();
    
    protected:
        void _notifyItemChange(int change);

    private:    
        ItemChangedNotifier* _toNotify = nullptr;
        QGraphicsItem* _item = nullptr;
        bool _mustNotify = true;
};