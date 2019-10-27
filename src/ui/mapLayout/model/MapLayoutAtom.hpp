#pragma once

#include "src/shared/models/RPZAtom.h"

#include "MapLayoutCategory.hpp"

class MapLayoutAtom : public MapLayoutItem {
    public:
        MapLayoutAtom(MapLayoutCategory* parent, const RPZAtom &atom) {
            this->_assetHash = atom.assetHash();
            this->_id = atom.id();
            this->_type = atom.type();
            this->updateFrom(atom.editedMetadataWithValues());
        }

        static MapLayoutAtom* fromIndex(const QModelIndex &index) {
            auto ip = index.internalPointer();
            return static_cast<MapLayoutAtom*>(ip);
        };

        void setParent(MapLayoutCategory* parent) {
            if(this->_parent) this->_parent->removeAsChild(this);
            this->_parent = parent;
        }

        MapLayoutCategory* parent() const {
            return this->_parent;
        }

        const QSet<AtomParameter> updateFrom(const AtomUpdates &updates) {
            
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
                    
                    case AtomParameter::AssetName:
                        this->setName(variant.toString());
                        break;

                    default:
                        break;
                }
            }

            return QSet<AtomParameter>::fromList(updates.keys());

        }

        bool isHidden() const {
            return this->_isHidden;
        }

        bool isLocked() const {
            return this->_isLocked;
        }
        
        const RPZAssetHash assetHash() const {
            return this->_assetHash;
        }
        
        const RPZAtomId atomId() const {
            return this->_id;
        }

        const QString name() const override {
            return this->_name;
        }

        void setName(const QString assetName) {
            this->_name = RPZAtom::toString(
                this->_type,
                assetName
            );
        }

        const QPixmap icon() const override { return QPixmap(); }

    private:
        MapLayoutCategory* _parent = nullptr;
        QString _name;
        RPZAtomType _type = (RPZAtomType)0;
        bool _isHidden = false;
        bool _isLocked = false;
        RPZAtomId _id = 0;
        RPZAssetHash _assetHash;

};