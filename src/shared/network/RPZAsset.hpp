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

        int layer() const {
            return (*this)["lyr"].toInt();
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

        void setLayer(int pos) {
            (*this)["lyr"] = pos;
        }
};

class RPZAsset : public AssetBase, public Serializable, public Ownable {
    public:
        RPZAsset() {}
        
        //enums
        enum Alteration {
            Unknown, 
            Resized, 
            Moved, 
            Added, 
            Removed, 
            Selected, 
            Focused, 
            Reset,
            LayerChange 
        }; 

        static inline const QList<Alteration> networkAlterations = { 
            Resized, 
            Moved, 
            Added, 
            Removed, 
            Reset,
            LayerChange 
        };
        static const inline QList<Alteration> buildGraphicsItemAlterations = {
            Added,  
            Reset
        };
        static const inline QList<Alteration> updateGraphicsItemAlterations = {
            Resized, 
            Moved, 
            LayerChange  
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

        static RPZAsset fromVariantHash(const QVariantHash &data) {
            return RPZAsset(
                data["id"].toUuid(),
                (AssetBase::Type)data["type"].toInt(),
                RPZUser::fromVariantHash(data["owner"].toHash()),
                data.contains("mdata") ? data["mdata"].toHash() : QVariantHash()
            );
        };

        void mayUpdateDataFromGraphicsItem(const Alteration &alteration) {
            
            if(!this->graphicsItem()) { return; }
            
            if(buildGraphicsItemAlterations.contains(alteration) || updateGraphicsItemAlterations.contains(alteration)) {
                this->_updateShapeFromGraphicsItem();
            }
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

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("type", (int)this->type());
            out.insert("mdata", this->_metadata);

            this->injectOwnerDataToHash(out);

            return out;
        };


    private:
        QGraphicsItem* _item = nullptr;
        RPZAssetMetadata _metadata;

        void _updateShapeFromGraphicsItem() {
            
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
                        this->graphicsItem()->boundingRect()
                    );
                }
                break;
            }

            //and position
            this->metadata()->setPos(
                this->graphicsItem()->scenePos()
            );

            //layer pos
            this->metadata()->setLayer(
                this->graphicsItem()->zValue()
            );
        }

};
