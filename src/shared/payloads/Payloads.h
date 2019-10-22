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
#include "local/AssetSelectedPayload.hpp"

class Payloads {
    public:
        static QSharedPointer<AlterationPayload> autoCast(const QVariantHash &hash) {
            
            AlterationPayload lowCast(hash);
            
            switch(lowCast.type()) {

                case PayloadAlteration::BulkMetadataChanged:
                    return QSharedPointer<BulkMetadataChangedPayload>(new BulkMetadataChangedPayload(hash));
                break;

                case PayloadAlteration::AtomTemplateChanged:
                    return QSharedPointer<AtomTemplateChangedPayload>(new AtomTemplateChangedPayload(hash));
                break;

                case PayloadAlteration::AtomTemplateSelected:
                    return QSharedPointer<AtomTemplateSelectedPayload>(new AtomTemplateSelectedPayload(hash));
                break;

                case PayloadAlteration::AssetChanged:
                    return QSharedPointer<AssetChangedPayload>(new AssetChangedPayload(hash));
                break;

                case PayloadAlteration::AssetSelected:
                    return QSharedPointer<AssetSelectedPayload>(new AssetSelectedPayload(hash));
                break;

                case PayloadAlteration::Added:
                    return QSharedPointer<AddedPayload>(new AddedPayload(hash));
                break;

                case PayloadAlteration::Reset:
                    return QSharedPointer<ResetPayload>(new ResetPayload(hash));
                break;

                case PayloadAlteration::Focused:
                    return QSharedPointer<FocusedPayload>(new FocusedPayload(hash));
                break;

                case PayloadAlteration::Removed:
                    return QSharedPointer<RemovedPayload>(new RemovedPayload(hash));
                break;

                case PayloadAlteration::Selected:
                    return QSharedPointer<SelectedPayload>(new SelectedPayload(hash));
                break;

                case PayloadAlteration::MetadataChanged:
                    return QSharedPointer<MetadataChangedPayload>(new MetadataChangedPayload(hash));
                break;

                default:
                    return QSharedPointer<AlterationPayload>(new AlterationPayload(hash));
                break;
                
            }

        }
};