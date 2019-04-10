#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Serializable.hpp"
#include "_serializer.hpp"
#include "Ownable.hpp"

#include "src/shared/map/MapHint.h"

#include "AssetBase.hpp"

class RPZAsset : public AssetBase, public Serializable, public Ownable {
    public:
        RPZAsset() {}
        
        
        //enums
        enum Alteration { Changed, Added, Removed, Selected, Focused, Reset };
        static inline const QList<Alteration> networkAlterations = { 
            Alteration::Changed, 
            Alteration::Added,
            Alteration::Removed 
        };
        static const inline QList<Alteration> mustParseGraphicsItem = {
            Alteration::Added, 
            Alteration::Reset,
            Alteration::Changed
        };

        RPZAsset(const QUuid &id, const AssetBase::Type &type, const QUuid &ownerId, const QString &ownerName, const QByteArray &data = NULL) : 
            Serializable(id), 
            AssetBase(type), 
            Ownable(ownerId, ownerName),
            _data(data) { };

        RPZAsset(const AssetBase::Type &type,  QGraphicsItem* assetItemOnMap) :
            Serializable(QUuid::createUuid()),
            AssetBase(type), 
            _item(assetItemOnMap) {  };


        QVariantHash toVariantHashWithData(const Alteration &alteration) {
            auto out = this->toVariantHash();
            
            //if no graphics item
            if(!this->graphicsItem() && this->_data.isNull()) {
                
                //warning
                qWarning() << "Trying to parse an asset with no GraphicsItem bound !";

            }

            //if graphicsItem bound, set data
            if(this->graphicsItem()) {

                //if alteration that required a parsing
                if(RPZAsset::mustParseGraphicsItem.contains(alteration)) {

                    //depending on assetType...
                    switch(this->type()) {
                        
                        //drawing...
                        case AssetBase::Type::Drawing:
                            auto casted = (QGraphicsPathItem*)this->graphicsItem();
                            const auto path = casted->path();
                            this->_data = JSONSerializer::toBase64(path);
                            break;
                    
                    }

                }

            }

            out.insert("data", this->_data);

            return out;
        }

        static RPZAsset fromVariantHash(const QVariantHash &data) {
            return RPZAsset(
                data["id"].toUuid(),
                (AssetBase::Type)data["type"].toInt(),
                data["oid"].toUuid(),
                data["oname"].toString(),
                data.contains("data") ? data["data"].toByteArray() : NULL
            );
        };

        QGraphicsItem* graphicsItem() { return this->_item; };
        void setGraphicsItem(QGraphicsItem* item) { this->_item = item; };
        QByteArray* data() { return &this->_data; };

        //overrides descriptor
        QString descriptor() override { 
            auto base = AssetBase::descriptor();

            if(!this->ownerName().isNull()) {
                base += " (" + this->ownerName() + ")";
            } else if (!this->ownerId().isNull()) {
                base += " (" + this->ownerId().toString() + ")";
            }

            return base;
        };


    private:
        QGraphicsItem* _item = nullptr;
        QByteArray _data;

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("type", (int)this->type());

            this->injectOwnerDataToHash(out);

            return out;
        };

};