#pragma once

#include "src/shared/models/RPZAtom.h"

typedef int RPZAtomLayer;

class MapLayoutItem {
    public:
        MapLayoutItem(const RPZAtom &atom) {
            this->_type = atom.type();
            this->_assetHash = atom.assetHash();
            this->_id = atom.id();
            this->updateFrom(atom.editedMetadataWithValues());
        }

        void updateFrom(const AtomUpdates &updates) {
            
            for(auto i = updates.begin(); i != updates.end(); i++) {
                
                auto param = i.key();
                auto variant = i.value();
                
                switch (param) {

                    case AtomParameter::Hidden:
                        this->_isHidden = variant.toBool();
                        break;
                    
                    case AtomParameter::Locked:
                        this->_isLocked = variant.toBool();
                        break;
                    
                    case AtomParameter::Layer:
                        this->_layer = variant.toInt();
                        break;
                    
                    case AtomParameter::AssetName:
                        this->_setName(variant.toString());
                        break;

                    default:
                        break;
                }
            }
            
        }

        bool isHidden() {
            return this->_isHidden;
        }

        bool isLocked() {
            return this->_isLocked;
        }
        
        const RPZAtomType type() {
            return this->_type;
        }

        const RPZAtomLayer layer() {
            return this->_layer;
        }

        const RPZAssetHash assetHash() {
            return this->_assetHash;
        }
        
        const RPZAtomId atomId() {
            return this->_id;
        }

        const QString name() {
            return this->name;
        }

    private:
        QString _name;
        bool _isHidden = false;
        bool _isLocked = false;
        int _layer = 0;
        RPZAtomType _type = RPZAtomType::Undefined;
        RPZAtomId _id = 0;
        RPZAssetHash _assetHash;

        void _setName(const QString assetName) {
            this->_name = RPZAtom::toString(
                this->type(),
                assetName
            );
        }
};