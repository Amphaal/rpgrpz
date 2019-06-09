#pragma once 

#include <QGraphicsItem>
#include <QString>
#include <QVector>

#include "base/Ownable.hpp"

#include "RPZAtomMetadata.hpp"

class RPZAtom : public Ownable, public RPZAtomMetadata {
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

        RPZAtom() : RPZAtomMetadata(this) {}
        RPZAtom(const QVariantHash &hash) : Ownable(hash), RPZAtomMetadata(this) {}
        RPZAtom(const snowflake_uid &id, const Type &type, const RPZUser &owner) : Ownable(id, owner), RPZAtomMetadata(this) {
            this->_setType(type);
        };
        RPZAtom(const Type &type) : Ownable(SnowFlake::get()->nextId()), RPZAtomMetadata(this) {
            this->_setType(type);  
        };

        QGraphicsItem* graphicsItem() { 
            return this->_graphicsItem; 
        };

        void setGraphicsItem(QGraphicsItem* item) { 
            this->_graphicsItem = item; 
        };

        Type type() {
            return (Type)this->value("type").toInt();
        }

        //overrides descriptor
        QString descriptor() { 

            //displays asset name
            auto asname = this->assetName();
            if(!asname.isNull()) return asname;

            return this->_defaultDescriptor();
        };


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
