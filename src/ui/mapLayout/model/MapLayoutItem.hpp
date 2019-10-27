#pragma once

#include <QModelIndex>
#include <QPixmap>

class MapLayoutItem {
    public:
        MapLayoutItem() {}
        virtual ~MapLayoutItem() {} //dynamic_cast

        static MapLayoutItem* fromIndex(const QModelIndex &index) {
            auto ip = index.internalPointer();
            return static_cast<MapLayoutItem*>(ip);
        };

        virtual const QString name() const = 0;
        virtual const QPixmap icon() const = 0;
};