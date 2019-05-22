#pragma once

#include <QString>

class AssetBase {
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

        AssetBase() {}
        AssetBase(const AssetBase::Type &type) : _selfType(type) {

            switch(type) {
                case Type::Drawing:
                    this->_description = "Dessin";
                    break;
                default:
                    this->_description = "Element";
            }

        } 
        
        AssetBase::Type type() const { return this->_selfType; }
        virtual QString descriptor() { return this->_description; }

    private:
        AssetBase::Type _selfType;
        QString _description;
};