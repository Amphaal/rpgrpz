#pragma once

#include <QSharedPointer>

#include "bulk/BulkMetadataChangedPayload.hpp"
#include "wielder/AddedPayload.hpp"
#include "wielder/ResetPayload.hpp"
#include "withTarget/FocusedPayload.hpp"
#include "withTarget/RemovedPayload.hpp"
#include "withTarget/SelectedPayload.hpp"
#include "withTarget/MetadataChangedPayload.hpp"
#include "local/AssetChangedPayload.hpp"
#include "local/AtomTemplateChangedPayload.hpp"
#include "local/AtomTemplateSelectedPayload.hpp"
#include "local/ToySelectedPayload.hpp"

class Payloads {
    public:
        static QSharedPointer<AlterationPayload> autoCast(const QVariantHash &hash) {
            
            AlterationPayload lowCast(hash);
            
            switch(lowCast.type()) {

                case Payload::Alteration::BulkMetadataChanged:
                    return QSharedPointer<BulkMetadataChangedPayload>(new BulkMetadataChangedPayload(hash));
                break;

                case Payload::Alteration::AtomTemplateChanged:
                    return QSharedPointer<AtomTemplateChangedPayload>(new AtomTemplateChangedPayload(hash));
                break;

                case Payload::Alteration::AtomTemplateSelected:
                    return QSharedPointer<AtomTemplateSelectedPayload>(new AtomTemplateSelectedPayload(hash));
                break;

                case Payload::Alteration::AssetChanged:
                    return QSharedPointer<AssetChangedPayload>(new AssetChangedPayload(hash));
                break;

                case Payload::Alteration::ToySelected:
                    return QSharedPointer<ToySelectedPayload>(new ToySelectedPayload(hash));
                break;

                case Payload::Alteration::Added:
                    return QSharedPointer<AddedPayload>(new AddedPayload(hash));
                break;

                case Payload::Alteration::Reset:
                    return QSharedPointer<ResetPayload>(new ResetPayload(hash));
                break;

                case Payload::Alteration::Focused:
                    return QSharedPointer<FocusedPayload>(new FocusedPayload(hash));
                break;

                case Payload::Alteration::Removed:
                    return QSharedPointer<RemovedPayload>(new RemovedPayload(hash));
                break;

                case Payload::Alteration::Selected:
                    return QSharedPointer<SelectedPayload>(new SelectedPayload(hash));
                break;

                case Payload::Alteration::MetadataChanged:
                    return QSharedPointer<MetadataChangedPayload>(new MetadataChangedPayload(hash));
                break;

                default:
                    return QSharedPointer<AlterationPayload>(new AlterationPayload(hash));
                break;
                
            }

        }
};