#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>
#include <QVector>

#include "../Serializable.hpp"
#include "../_serializer.hpp"
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
            Alteration::Reset
        };

        RPZAsset(const QUuid &id, const AssetBase::Type &type, const RPZUser &owner, const QByteArray &data = NULL, const QVariantHash &metadata = QVariantHash()) : 
            Serializable(id), 
            AssetBase(type), 
            Ownable(owner),
            _data(data),
            _metadata(metadata) { };

        RPZAsset(const AssetBase::Type &type,  QGraphicsItem* assetItemOnMap, const QVariantHash &metadata = QVariantHash()) :
            Serializable(QUuid::createUuid()),
            AssetBase(type), 
            _item(assetItemOnMap),
            _metadata(metadata) {  };


        QVariantHash toVariantHashWithData(const Alteration &alteration) {
            
            //initial
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
                RPZUser::fromVariantHash(data["owner"].toHash()),
                data.contains("data") ? data["data"].toByteArray() : NULL,
                data.contains("mdata") ? data["mdata"].toHash() : QVariantHash()
            );
        };

        QGraphicsItem* graphicsItem() { return this->_item; };
        void setGraphicsItem(QGraphicsItem* item) { this->_item = item; };
        QByteArray* data() { return &this->_data; };
        QVariantHash* metadata() { return &this->_metadata; };

        //overrides descriptor
        QString descriptor() override { 
            auto base = AssetBase::descriptor();

            if(!this->owner().name().isNull()) {
                base += " (" + this->owner().name() + ")";
            } else if (!this->owner().id().isNull()) {
                base += " (" + this->owner().id().toString() + ")";
            }

            return base;
        };


    private:
        QGraphicsItem* _item = nullptr;
        QByteArray _data;
        QVariantHash _metadata;

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("type", (int)this->type());
            out.insert("mdata", this->_metadata);

            this->injectOwnerDataToHash(out);

            return out;
        };

};