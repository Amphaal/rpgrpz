#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QUuid>
#include <QVector>

#include "base/Ownable.hpp"

#include "RPZAtomMetadata.hpp"

class RPZAtom : public Ownable {
    public:

        // defined values shared with AssetsDatabaseElement type for static casts
        enum Type { 
            Undefined, 
            Drawing,
            Text,
            Object = 105, 
            Brush = 104, 
            NPC = 103, 
            Event = 102, 
            PC = 101 
        };

        RPZAtom() {}
        RPZAtom(const QVariantHash &hash) : Ownable(hash) {}

        RPZAtom(const QUuid &id, const Type &type, const RPZUser &owner, const RPZAtomMetadata &metadata) : Ownable(id, owner) {
                this->_setType(type);
                this->_setMetadata(metadata);
            };

        RPZAtom(const Type &type, QGraphicsItem* item, const RPZAtomMetadata &metadata) : Ownable(QUuid::createUuid()), _item(item){
                this->_setType(type);  
                this->_setMetadata(metadata);
            };

        QGraphicsItem* graphicsItem() { 
            return this->_item; 
        };

        void setGraphicsItem(QGraphicsItem* item) { 
            this->_item = item; 
        };

        RPZAtomMetadata* metadata() { 
            auto metadata = (RPZAtomMetadata)this->value("mdata").toHash(); 
            return &metadata;
        };

        Type type() {
            return (Type)this->value("type").toInt();
        }

        //overrides descriptor
        QString descriptor() { 

            //displays asset name
            auto asname = this->metadata()->assetName();
            if(!asname.isNull()) return asname;

            return this->_defaultDescriptor();
        };


    private:
        QGraphicsItem* _item = nullptr;

        QString _defaultDescriptor() {
            switch(this->type()) {
                case Type::Drawing:
                    return "Dessin";
                    break;
                default:
                    return "Atome";
            }
        }

        void _setMetadata(const RPZAtomMetadata &metadata) {
            (*this)["mdata"] = metadata;
        }

        void _setType(const Type &type) {
            (*this)["type"] = (int)type;
        }
};
