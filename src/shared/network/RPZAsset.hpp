#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>
#include <QVector>

#include "../Serializable.hpp"
#include "../_serializer.hpp"
#include "Ownable.hpp"

#include "AssetBase.hpp"

class RPZAsset : public AssetBase, public Serializable, public Ownable {
    public:
        RPZAsset() {}
        
        //enums
        enum Alteration { 
            Resized, 
            Moved, 
            Added, 
            Removed, 
            Selected, 
            Focused, 
            Reset 
        }; 

        static inline const QList<Alteration> networkAlterations = { 
            Resized, 
            Moved, 
            Added, 
            Removed, 
            // Selected, 
            // Focused, 
            Reset 
        };
        static const inline QList<Alteration> buildGraphicsItemAlterations = {
            // Resized, 
            // Moved, 
            Added, 
            // Removed, 
            // Selected, 
            // Focused, 
            Reset 
        };
        static const inline QList<Alteration> updateGraphicsItemAlterations = {
            Resized, 
            Moved, 
            // Added, 
            // Removed, 
            // Selected, 
            // Focused, 
            // Reset 
        };

        RPZAsset(const QUuid &id, const AssetBase::Type &type, const RPZUser &owner, const QByteArray &shape = NULL, const QVariantHash &metadata = QVariantHash()) : 
            Serializable(id), 
            AssetBase(type), 
            Ownable(owner),
            _shape(shape),
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
            if(!this->graphicsItem() && this->_shape.isNull()) {
                
                //warning
                qWarning() << "Trying to parse an asset with no GraphicsItem bound !";

            }

            //if graphicsItem bound, set data
            if(this->graphicsItem()) {

                //if alteration that required a parsing
                if(RPZAsset::buildGraphicsItemAlterations.contains(alteration)) {
                    this->updateShapeFromGraphicsItem();
                }

            }

            out.insert("shape", this->_shape);

            return out;
        }

        static RPZAsset fromVariantHash(const QVariantHash &data) {
            return RPZAsset(
                data["id"].toUuid(),
                (AssetBase::Type)data["type"].toInt(),
                RPZUser::fromVariantHash(data["owner"].toHash()),
                data.contains("shape") ? data["shape"].toByteArray() : NULL,
                data.contains("mdata") ? data["mdata"].toHash() : QVariantHash()
            );
        };

        void updateShapeFromGraphicsItem() {
            
            if(!this->graphicsItem()) return;

            //depending on assetType...
            switch(this->type()) {
                
                //drawing...
                case AssetBase::Type::Drawing: {
                    auto casted = (QGraphicsPathItem*)this->graphicsItem();
                    const auto path = casted->path();
                    this->_shape = JSONSerializer::toBase64(path);
                }
                break;
                
                //object
                case AssetBase::Type::Object: {
                    QPainterPath shape;
                    shape.addRect(this->graphicsItem()->sceneBoundingRect());
                    this->_shape = JSONSerializer::toBase64(shape);
                }
                break;
            
            }
        }

        QGraphicsItem* graphicsItem() { return this->_item; };
        void setGraphicsItem(QGraphicsItem* item) { this->_item = item; };
        QByteArray* shape() { return &this->_shape; };
        QVariantHash* metadata() { return &this->_metadata; };

        //overrides descriptor
        QString descriptor() override { 
            auto base = AssetBase::descriptor();

            //displays asset name
            auto asname = this->_metadata["a_name"].toString();
            if(!asname.isNull()) {
                base += " \"" + asname + "\" ";
            }

            //displays ownership
            if(!this->owner().name().isNull()) {
                base += " (" + this->owner().name() + ")";
            } else if (!this->owner().id().isNull()) {
                base += " (" + this->owner().id().toString() + ")";
            }

            return base;
        };


    private:
        QGraphicsItem* _item = nullptr;
        QByteArray _shape;
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