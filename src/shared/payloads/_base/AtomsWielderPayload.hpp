#pragma once

#include "src/shared/models/_base/RPZMap.hpp"
#include "src/shared/models/RPZAtom.h"

#include "AlterationPayload.hpp"

#include "src/shared/database/MapDatabase.h"

class AtomsWielderPayload : public AlterationPayload {
    public:
        AtomsWielderPayload() {}
        AtomsWielderPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomsWielderPayload(const PayloadAlteration &alteration, const MapDatabase &map) : AlterationPayload(alteration) {
            this->_setAssetsIds(map);
            this->_setAtoms(map);
        }

        const QSet<RPZAssetHash> assetIds() const {
            QSet<RPZAssetHash> out;
        
            for(auto &i : this->_assetsIds()) out += i.toString();
            
            return out;
        }
            
        RPZMap<RPZAtom> atoms() const {
            RPZMap<RPZAtom> out;
            
            auto map = this->_atoms();
            for(auto i = map.begin(); i != map.end(); ++i) {    
                
                auto snowflakeId = i.key().toULongLong();
                RPZAtom atom(i.value().toHash());
                out.insert(snowflakeId, atom);

            }
            
            return out;
        }

        private:
            QVariantList _assetsIds() const {
                return this->value(QStringLiteral(u"assets")).toList();
            }

            QVariantMap _atoms() const {
                return this->value(QStringLiteral(u"atoms")).toMap();
            }

            void _setAssetsIds(const MapDatabase &map) {
                
                QVariantList vList;
                auto assetsIds = map.safe_usedAssetsIds();
                
                for (auto &hash : assetsIds) {
                    vList += hash;
                }
                this->insert(QStringLiteral(u"assets"), vList);
                
            }

            void _setAtoms(const MapDatabase &map) {
                
                QVariantMap vMap;
                auto atoms = map.safe_atoms();

                for (auto i = atoms.constBegin(); i != atoms.constEnd(); ++i) {
                    auto snowflakeAsStr = QString::number(i.key());
                    auto maybePartialAtom = i.value();
                    vMap.insert(snowflakeAsStr, maybePartialAtom);
                }

                this->insert(QStringLiteral(u"atoms"), vMap);

            }

};