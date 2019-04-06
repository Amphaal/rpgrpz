#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>

#include "AssetType.hpp"

class Asset : public AssetType {
    public:
        Asset() {}
        Asset(AssetType::Type type, QGraphicsItem* assetItemOnMap, QUuid assetId = NULL, QUuid ownerId = NULL, QString ownerName = NULL) :
            AssetType(type), 
            _item(assetItemOnMap),
            _ownerId(ownerId),
            _ownerName(ownerName),
            _id(assetId) { 

        }

        QGraphicsItem* graphicsItem() { return _item; }
        QUuid ownerId() { return _ownerId; }
        QUuid id() { return _id; }
        void setId(QUuid id) { _id = id; }

        QString descriptor() override { 
            auto base = AssetType::descriptor();

            if(!this->_ownerName.isNull()) {
                base += "(" + this->_ownerName + ")";
            } else if (!this->_ownerId.isNull()) {
                base += "(" + this->_ownerId.toString() + ")";
            }

            return base;
        } 


    private:
        QGraphicsItem * _item;
        QUuid _ownerId;
        QUuid _id;
        QString _ownerName;
};