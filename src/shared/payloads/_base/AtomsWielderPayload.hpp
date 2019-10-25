#pragma once

#include "src/shared/models/_base/RPZMap.hpp"
#include "src/shared/models/RPZAtom.h"

#include "AlterationPayload.hpp"

#include "src/shared/database/MapDatabase.h"

class AtomsWielderPayload : public AlterationPayload {
    public:
        AtomsWielderPayload() {}

        const QSet<RPZAssetHash> assetHashes() const {
            
            QSet<RPZAssetHash> out;

            auto hashes = this->value(QStringLiteral(u"assets")).toList();

            for(auto &i : hashes) out += i.toString();
            
            return out;

        }
            
        const RPZMap<RPZAtom> atoms() const {
            
            RPZMap<RPZAtom> out;
            
            auto atoms = this->value(QStringLiteral(u"atoms")).toMap();
            
            for(auto i = atoms.begin(); i != atoms.end(); ++i) {    
                
                auto snowflakeId = i.key().toULongLong();
                RPZAtom atom(i.value().toHash());
                out.insert(snowflakeId, atom);

            }
            
            return out;

        }

        protected:
        explicit AtomsWielderPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomsWielderPayload(const MapDatabase &map) : AlterationPayload(Payload::Alteration::Reset) {
            this->_setAssetHashes(map.usedAssetHashes());
            this->_setAtoms(map.atoms().toVMap());
        }

        AtomsWielderPayload(const QList<RPZAtom> &atoms) : AlterationPayload(Payload::Alteration::Added) {
            this->_setAssetHashes(atoms);
            this->_setAtoms(atoms);
        }

        private:

            void _setAssetHashes(const QList<RPZAtom> &toExtractFrom) {
                
                QVariantList vList;
                
                for (auto &atom : toExtractFrom) {
                    vList += atom.assetHash();
                }

                this->_setAssetHashes(vList);
                
            }

            void _setAssetHashes(const QSet<RPZAssetHash> &hashes) {
                
                QVariantList vList;
                
                for (auto &hash : hashes) {
                    vList += hash;
                }

                this->_setAssetHashes(vList);
                
            }

            void _setAtoms(const QList<RPZAtom> &atoms) {
                
                QVariantMap vMap;

                for (auto &atom : atoms) {
                    vMap.insert(atom.idAsStr(), atom);
                }

                this->_setAtoms(vMap);

            }

            void _setAssetHashes(const QVariantList &hashes) {
                this->insert(QStringLiteral(u"assets"), hashes);
            }

            void _setAtoms(const QVariantMap &atoms) {
                this->insert(QStringLiteral(u"atoms"), atoms);
            }

};