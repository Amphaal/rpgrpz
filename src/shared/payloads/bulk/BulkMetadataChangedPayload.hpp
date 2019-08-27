#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

typedef QHash<snowflake_uid, AtomUpdates> AtomsUpdates;

class BulkMetadataChangedPayload : public AlterationPayload {
    public:
        BulkMetadataChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) { }
        BulkMetadataChangedPayload(const AtomsUpdates &changes) : AlterationPayload(PayloadAlteration::PA_BulkMetadataChanged) { 
            this->_defineAtomsUpdates(changes);
        }

        AtomsUpdates atomsUpdates() const {
            auto rawData = this->value("changes").toHash();
            AtomsUpdates out;

            for(auto i = rawData.constBegin(); i != rawData.constEnd(); i++) {
                
                snowflake_uid atomId = i.key().toULongLong();
                auto changes = i.value().toHash();
                AtomUpdates formatedAtoms;

                for(auto y = changes.begin(); y != changes.end(); y++) {
                    
                    AtomParameter param = (AtomParameter)y.key().toInt();

                    formatedAtoms.insert(param, y.value());
                }

                out.insert(atomId, formatedAtoms);

            }

            return out;
        }
    
    private:
        void _defineAtomsUpdates(const AtomsUpdates &changes) {
            
            QVariantHash hash;
            
            for(auto i = changes.constBegin(); i != changes.constEnd(); i++) {
                
                auto idStr = QString::number(i.key());
                QVariantHash changesById;

                for(auto y = i.value().constBegin(); y != i.value().constEnd(); y++) {
                    auto paramStr = QString::number(y.key());

                    changesById.insert(paramStr, y.value());
                }

                hash.insert(idStr, changesById);

            }

            this->insert("changes", hash);
        }
};