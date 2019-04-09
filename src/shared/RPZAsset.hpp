#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>

#include "AssetBase.hpp"

class RPZAsset : public AssetBase {
    public:
        RPZAsset() {}
        RPZAsset(const AssetBase::Type &type, QGraphicsItem* assetItemOnMap, 
                    const QUuid &assetId = NULL, const QUuid &ownerId = NULL, const QString &ownerName = NULL) :
            AssetBase(type), 
            _item(assetItemOnMap),
            _ownerId(ownerId),
            _ownerName(ownerName),
            _id(assetId) { 

        }

        QGraphicsItem* graphicsItem() { return this->_item; }
        QUuid ownerId() const { return this->_ownerId; }
        QUuid id() const { return this->_id; }
        void setId(const QUuid &id) { this->_id = id; }

        QString descriptor() const override { 
            auto base = AssetBase::descriptor();

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