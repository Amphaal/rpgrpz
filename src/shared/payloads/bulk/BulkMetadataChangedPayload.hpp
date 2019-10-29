#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"


class BulkMetadataChangedPayload : public AlterationPayload {
    public:
        explicit BulkMetadataChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) { }
        BulkMetadataChangedPayload(const RPZAtom::ManyUpdates &changes) : AlterationPayload(Payload::Alteration::BulkMetadataChanged) { 
            this->_defineAtomsUpdates(changes);
        }

        RPZAtom::ManyUpdates atomsUpdates() const {
            
            auto rawData = this->value(QStringLiteral(u"changes")).toHash();
            RPZAtom::ManyUpdates out;

            for(auto i = rawData.constBegin(); i != rawData.constEnd(); i++) {

                out.insert(
                    i.key().toULongLong(),
                    RPZAtom::unserializeUpdates(i.value().toHash())
                );

            }

            return out;
            
        }
    
    private:
        void _defineAtomsUpdates(const RPZAtom::ManyUpdates &changes) {
            
            QVariantHash hash;
            
            for(auto i = changes.constBegin(); i != changes.constEnd(); i++) {
                
                auto idStr = QString::number(i.key());
                QVariantHash changesById;

                hash.insert(
                    idStr, 
                    RPZAtom::serializeUpdates(i.value())
                );

            }

            this->insert(QStringLiteral(u"changes"), hash);
        }
};