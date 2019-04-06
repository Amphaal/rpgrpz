#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>

#include "AssetType.hpp"

class Asset : public AssetType {
    public:
        Asset() {}
        Asset(const AssetType::Type &type, QGraphicsItem* assetItemOnMap, 
                    const QUuid &assetId = NULL, const QUuid &ownerId = NULL, const QString &ownerName = NULL) :
            AssetType(type), 
            _item(assetItemOnMap),
            _ownerId(ownerId),
            _ownerName(ownerName),
            _id(assetId) { 

        }

        QGraphicsItem* graphicsItem() { return _item; }
        QUuid ownerId() const { return _ownerId; }
        QUuid id() const { return _id; }
        void setId(const QUuid &id) { _id = id; }

        QString descriptor() const override { 
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