#pragma once 

#include <QGraphicsItem>
#include <QString>
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

        RPZAtom(const snowflake_uid &id, const Type &type, const RPZUser &owner, const RPZAtomMetadata &metadata) : Ownable(id, owner) {
                this->_setType(type);
                this->setMetadata(metadata);
            };

        RPZAtom(const Type &type, const RPZAtomMetadata &metadata) : Ownable(SnowFlake::get()->nextId()) {
                this->_setType(type);  
                this->setMetadata(metadata);
            };

        QGraphicsItem* graphicsItem() { 
            return this->_graphicsItem; 
        };

        void setGraphicsItem(QGraphicsItem* item) { 
            this->_graphicsItem = item; 
        };

        RPZAtomMetadata metadata() { 
            return (RPZAtomMetadata)this->value("mdata").toHash();
        };

        Type type() {
            return (Type)this->value("type").toInt();
        }

        //overrides descriptor
        QString descriptor() { 

            //displays asset name
            auto asname = this->metadata().assetName();
            if(!asname.isNull()) return asname;

            return this->_defaultDescriptor();
        };

        void setMetadata(const RPZAtomMetadata &metadata) {
            (*this)["mdata"] = metadata;
        }


    private:
        QGraphicsItem* _graphicsItem = nullptr;

        QString _defaultDescriptor() {
            switch(this->type()) {
                case Type::Drawing:
                    return "Dessin";
                    break;
                case Type::Text:
                    return "Texte";
                    break;
                default:
                    return "Atome";
            }
        }

        void _setType(const Type &type) {
            (*this)["type"] = (int)type;
        }
};
