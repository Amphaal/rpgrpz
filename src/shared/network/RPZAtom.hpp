#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>
#include <QVector>

#include "../Serializable.hpp"
#include "../_serializer.hpp"
#include "Ownable.hpp"

#include "AtomBase.hpp"
#include "RPZAtomMetadata.hpp"

class RPZAtom : public AtomBase, public Serializable, public Ownable {
    public:
        RPZAtom() {}
        
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

        RPZAtom(const QUuid &id, const AtomBase::Type &type, const RPZUser &owner, const RPZAtomMetadata &metadata) : 
            Serializable(id), 
            AtomBase(type), 
            Ownable(owner),
            _metadata(metadata) { };

        RPZAtom(const AtomBase::Type &type,  QGraphicsItem* item, const RPZAtomMetadata &metadata) :
            Serializable(QUuid::createUuid()),
            AtomBase(type), 
            _item(item),
            _metadata(metadata) {  };

        static RPZAtom fromVariantHash(const QVariantHash &data) {
            return RPZAtom(
                data["id"].toUuid(),
                (AtomBase::Type)data["type"].toInt(),
                RPZUser::fromVariantHash(data["owner"].toHash()),
                data.contains("mdata") ? data["mdata"].toHash() : QVariantHash()
            );
        };

        void mayUpdateDataFromGraphicsItem(const Alteration &alteration) {
            
            if(!this->graphicsItem()) return;
            
            if(buildGraphicsItemAlterations.contains(alteration) || updateGraphicsItemAlterations.contains(alteration)) {
                this->_updateShapeFromGraphicsItem();
            }
        }

        QGraphicsItem* graphicsItem() { return this->_item; };
        void setGraphicsItem(QGraphicsItem* item) { this->_item = item; };

        RPZAtomMetadata* metadata() { return &this->_metadata; };

        //overrides descriptor
        QString descriptor() override { 
            auto base = AtomBase::descriptor();

            //displays asset name
            auto asname = this->metadata()->assetName();
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
        RPZAtomMetadata _metadata;

        void _updateShapeFromGraphicsItem() {
            
            //depending on atomType...
            switch(this->type()) {
                
                //drawing...
                case AtomBase::Type::Drawing: {
                    auto casted = (QGraphicsPathItem*)this->graphicsItem();
                    this->metadata()->setShape(
                        casted->path()
                    );
                }
                break;
                
                //object
                case AtomBase::Type::Object: {
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
