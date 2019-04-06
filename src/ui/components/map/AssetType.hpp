#pragma once

class AssetType {
    public:
        enum Type { Undefined, Drawing, Brush, NPC, Event, PC };

        AssetType() {}
        AssetType(AssetType::Type type) : _selfType(type) {

            switch(type) {
                case Type::Drawing:
                    this->_description = "Dessin";
                    break;
                default:
                    this->_description = "Element";
            }

        } 
        
        AssetType::Type type() { return this->_selfType; }
        virtual QString descriptor() { return this->_description; }

    private:
        AssetType::Type _selfType;
        QString _description;
};