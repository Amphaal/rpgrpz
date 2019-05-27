#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>
#include <QVector>

#include "../Serializable.hpp"
#include "../_serializer.hpp"
#include "Ownable.hpp"

#include "AssetBase.hpp"


class RPZAssetMetadata : public QVariantHash {
    public:
        RPZAssetMetadata() : QVariantHash() {}
        RPZAssetMetadata(const QVariantHash &hash) : QVariantHash(hash) {}
    
        QString dbAssetId() const {
            return (*this)["a_id"].toString();
        }

        QString dbAssetName() const {
            return (*this)["a_name"].toString();
        }

        QPainterPath shape() const {
            return JSONSerializer::toPainterPath(
                (*this)["shape"].toByteArray()
            );
        }

        QPointF pos() const {
            return JSONSerializer::toPointF(
                (*this)["pos"].toByteArray()
            );
        }

        int penWidth() const {
            return (*this)["pen_w"].toInt();
        }

        void setDbAssetId(const QString &id) {
            (*this)["a_id"] = id;
        }

        void setDbAssetName(const QString &name) {
            (*this)["a_name"] = name;
        }

        void setShape(const QPainterPath &path) {
            (*this)["shape"] = JSONSerializer::asBase64(path);
        }

        void setShape(const QRectF &rect) {
            QPainterPath shape;
            shape.addRect(rect);
            this->setShape(shape);
        }

        void setPos(const QPointF &pos) {
            (*this)["pos"] = JSONSerializer::asBase64(pos);
        }

        void setPenWidth(int width) {
            (*this)["pen_w"] = width;
        }
};

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

        RPZAsset(const QUuid &id, const AssetBase::Type &type, const RPZUser &owner, const RPZAssetMetadata &metadata) : 
            Serializable(id), 
            AssetBase(type), 
            Ownable(owner),
            _metadata(metadata) { };

        RPZAsset(const AssetBase::Type &type,  QGraphicsItem* assetItemOnMap, const RPZAssetMetadata &metadata) :
            Serializable(QUuid::createUuid()),
            AssetBase(type), 
            _item(assetItemOnMap),
            _metadata(metadata) {  };


        QVariantHash toVariantHashWithData(const Alteration &alteration) {
            
            //if graphicsItem bound, set data
            if(this->graphicsItem()) {

                //if alteration that required a parsing
                if(RPZAsset::buildGraphicsItemAlterations.contains(alteration)) {
                    this->updateShapeFromGraphicsItem();
                }

            } else {
                qWarning() << "Trying to parse an asset with no GraphicsItem bound !";
            }

            return this->toVariantHash();
        }

        static RPZAsset fromVariantHash(const QVariantHash &data) {
            return RPZAsset(
                data["id"].toUuid(),
                (AssetBase::Type)data["type"].toInt(),
                RPZUser::fromVariantHash(data["owner"].toHash()),
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
                    this->metadata()->setShape(
                        casted->path()
                    );
                }
                break;
                
                //object
                case AssetBase::Type::Object: {
                    this->metadata()->setShape(
                        this->graphicsItem()->sceneBoundingRect()
                    );
                }
                break;
            }

            this->metadata()->setPos(
                this->graphicsItem()->scenePos()
            );

        }

        QGraphicsItem* graphicsItem() { return this->_item; };
        void setGraphicsItem(QGraphicsItem* item) { this->_item = item; };

        RPZAssetMetadata* metadata() { return &this->_metadata; };

        //overrides descriptor
        QString descriptor() override { 
            auto base = AssetBase::descriptor();

            //displays asset name
            auto asname = this->metadata()->dbAssetName();
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
        RPZAssetMetadata _metadata;

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("type", (int)this->type());
            out.insert("mdata", this->_metadata);

            this->injectOwnerDataToHash(out);

            return out;
        };

};
