#pragma once 

#include <QGraphicsItem>

#include "AssetType.hpp"

class Asset : public AssetType {
    public:
        Asset() {}
        Asset(AssetType::Type type, QGraphicsItem * assetItemOnMap) : AssetType(type), _item(assetItemOnMap) { }

        QGraphicsItem * graphicsItem() { return _item; }

    private:
        QGraphicsItem * _item;
};