#pragma once

class AssetBase {
    public:
        enum Type { Undefined, Drawing, Brush, NPC, Event, PC };

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
        virtual QString descriptor() const { return this->_description; }

    private:
        AssetBase::Type _selfType;
        QString _description;
};