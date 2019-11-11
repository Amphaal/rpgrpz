#pragma once

#include "src/shared/payloads/_base/AtomRelatedPayload.hpp"


class BulkMetadataChangedPayload : public AtomRelatedPayload {
    public:
        explicit BulkMetadataChangedPayload(const QVariantHash &hash) : AtomRelatedPayload(hash) { }
        BulkMetadataChangedPayload(const RPZAtom::ManyUpdates &changes) : AtomRelatedPayload(Payload::Alteration::BulkMetadataChanged) { 
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

        AtomRelatedPayload::RemainingAtomIds restrictTargetedAtoms(const QSet<RPZAtom::Id> &idsToRemove) override {
            
            auto remainingUpdates = this->atomsUpdates();

            for(const auto &id : idsToRemove) {
                remainingUpdates.remove(id);
            }

            this->_defineAtomsUpdates(remainingUpdates);

            return remainingUpdates.count();

        };
    
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