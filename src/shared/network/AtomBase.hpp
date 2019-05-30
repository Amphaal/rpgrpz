#pragma once

#include <QString>

class AtomBase {
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

        AtomBase() {}
        AtomBase(const AtomBase::Type &type) : _selfType(type) {

            switch(type) {
                case Type::Drawing:
                    this->_description = "Dessin";
                    break;
                default:
                    this->_description = "Element";
            }

        } 
        
        AtomBase::Type type() const { return this->_selfType; }
        virtual QString descriptor() { return this->_description; }

    private:
        AtomBase::Type _selfType;
        QString _description;
};