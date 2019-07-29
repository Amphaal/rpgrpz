#pragma once

#include <QSharedPointer>

#include "wielder/BulkMetadataChangedPayload.hpp"
#include "wielder/AddedPayload.hpp"
#include "wielder/ResetPayload.hpp"
#include "withTarget/FocusedPayload.hpp"
#include "withTarget/RemovedPayload.hpp"
#include "withTarget/SelectedPayload.hpp"
#include "withTarget/DuplicatedPayload.hpp"
#include "withTarget/OwnerChangedPayload.hpp"
#include "withTarget/MetadataChangedPayload.hpp"
#include "basic/RedonePayload.hpp"
#include "basic/UndonePayload.hpp"

class Payloads {
    public:
        static QSharedPointer<AlterationPayload> autoCast(const QVariantHash &hash) {
            
            auto lowCast = AlterationPayload(hash);
            
            switch(lowCast.type()) {
                
                case PayloadAlteration::PA_BulkMetadataChanged:
                    return QSharedPointer<BulkMetadataChangedPayload>(new BulkMetadataChangedPayload(hash));
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

                case PayloadAlteration::PA_Duplicated:
                    return QSharedPointer<DuplicatedPayload>(new DuplicatedPayload(hash));
                break;

                case PayloadAlteration::PA_OwnerChanged:
                    return QSharedPointer<OwnerChangedPayload>(new OwnerChangedPayload(hash));
                break;

                case PayloadAlteration::PA_MetadataChanged:
                    return QSharedPointer<MetadataChangedPayload>(new MetadataChangedPayload(hash));
                break;

                case PayloadAlteration::PA_Redone:
                    return QSharedPointer<RedonePayload>(new RedonePayload(hash));
                break;

                case PayloadAlteration::PA_Undone:
                    return QSharedPointer<UndonePayload>(new UndonePayload(hash));
                break;

                default:
                    return QSharedPointer<AlterationPayload>(new AlterationPayload(hash));
                break;
                
            }

        }
};