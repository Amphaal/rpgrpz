#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

typedef QHash<RPZAtomId, AtomUpdates> AtomsUpdates;

class BulkMetadataChangedPayload : public AlterationPayload {
    public:
        BulkMetadataChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) { }
        BulkMetadataChangedPayload(const AtomsUpdates &changes) : AlterationPayload(PayloadAlteration::PA_BulkMetadataChanged) { 
            this->_defineAtomsUpdates(changes);
        }

        AtomsUpdates atomsUpdates() const {
            auto rawData = this->value(QStringLiteral(u"changes")).toHash();
            AtomsUpdates out;

            for(auto i = rawData.constBegin(); i != rawData.constEnd(); i++) {
                
                RPZAtomId RPZAtomId = i.key().toULongLong();
                
                out.insert(RPZAtomId, 
                    JSONSerializer::unserializeUpdates(i.value().toHash())
                );

            }

            return out;
        }
    
    private:
        void _defineAtomsUpdates(const AtomsUpdates &changes) {
            
            QVariantHash hash;
            
            for(auto i = changes.constBegin(); i != changes.constEnd(); i++) {
                
                auto idStr = QString::number(i.key());
                QVariantHash changesById;

                hash.insert(idStr, 
                    JSONSerializer::serializeUpdates(i.value())
                );

            }

            this->insert(QStringLiteral(u"changes"), hash);
        }
};