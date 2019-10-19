#pragma once

#include "src/shared/models/base/RPZMap.hpp"
#include "src/shared/models/RPZAtom.h"

#include "AlterationPayload.hpp"

class AtomsWielderPayload : public AlterationPayload {
    public:
        AtomsWielderPayload() {}
        AtomsWielderPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomsWielderPayload(const PayloadAlteration &alteration, const RPZMap<RPZAtom> &atoms, const QSet<RPZAssetHash> &includedAssetIds) : AlterationPayload(alteration) {
            
            this->_setAddedAtoms(atoms);

            QVariantList assetIds;
            for(auto &i : includedAssetIds) assetIds += i;
            this->insert(QStringLiteral(u"assets"), assetIds);

        }

        const QSet<RPZAssetHash> assetIds() const {
            QSet<RPZAssetHash> out;

            for(auto &i : this->value(QStringLiteral(u"assets")).toList()) out += i.toString();
            
            return out;
        }
            
        RPZMap<RPZAtom> atoms() const {
            RPZMap<RPZAtom> out;
            
            auto map = this->value(QStringLiteral(u"atoms")).toMap();

            for(auto i = map.begin(); i != map.end(); ++i) {    
                auto snowflakeId = i.key().toULongLong();
                RPZAtom atom(i.value().toHash());
                out.insert(snowflakeId, atom);
            }
            
            return out;
        }

    private:
        void _setAddedAtoms(const RPZMap<RPZAtom> &atoms) {
            QVariantMap list;
            for (RPZMap<RPZAtom>::const_iterator i = atoms.constBegin(); i != atoms.constEnd(); ++i) {
                auto snowflakeAsStr = QString::number(i.key());
                auto maybePartialAtom = i.value();
                list.insert(snowflakeAsStr, maybePartialAtom);
            }
            this->insert(QStringLiteral(u"atoms"), list);
        }
};