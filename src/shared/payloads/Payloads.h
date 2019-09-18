#pragma once

#include <QSharedPointer>

#include "bulk/BulkMetadataChangedPayload.hpp"
#include "wielder/AddedPayload.hpp"
#include "wielder/ResetPayload.hpp"
#include "withTarget/FocusedPayload.hpp"
#include "withTarget/RemovedPayload.hpp"
#include "withTarget/SelectedPayload.hpp"
#include "withTarget/OwnerChangedPayload.hpp"
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

                case PayloadAlteration::PA_BulkMetadataChanged:
                    return QSharedPointer<BulkMetadataChangedPayload>(new BulkMetadataChangedPayload(hash));
                break;

                case PayloadAlteration::PA_AtomTemplateChanged:
                    return QSharedPointer<AtomTemplateChangedPayload>(new AtomTemplateChangedPayload(hash));
                break;

                case PayloadAlteration::PA_AtomTemplateSelected:
                    return QSharedPointer<AtomTemplateSelectedPayload>(new AtomTemplateSelectedPayload(hash));
                break;

                case PayloadAlteration::PA_AssetChanged:
                    return QSharedPointer<AssetChangedPayload>(new AssetChangedPayload(hash));
                break;

                case PayloadAlteration::PA_AssetSelected:
                    return QSharedPointer<AssetSelectedPayload>(new AssetSelectedPayload(hash));
                break;

                case PayloadAlteration::PA_Added:
                    return QSharedPointer<AddedPayload>(new AddedPayload(hash));
                break;

                case PayloadAlteration::PA_Reset:
                    return QSharedPointer<ResetPayload>(new ResetPayload(hash));
                break;

                case PayloadAlteration::PA_Focused:
                    return QSharedPointer<FocusedPayload>(new FocusedPayload(hash));
                break;

                case PayloadAlteration::PA_Removed:
                    return QSharedPointer<RemovedPayload>(new RemovedPayload(hash));
                break;

                case PayloadAlteration::PA_Selected:
                    return QSharedPointer<SelectedPayload>(new SelectedPayload(hash));
                break;

                case PayloadAlteration::PA_OwnerChanged:
                    return QSharedPointer<OwnerChangedPayload>(new OwnerChangedPayload(hash));
                break;

                case PayloadAlteration::PA_MetadataChanged:
                    return QSharedPointer<MetadataChangedPayload>(new MetadataChangedPayload(hash));
                break;

                default:
                    return QSharedPointer<AlterationPayload>(new AlterationPayload(hash));
                break;
                
            }

        }
};