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
        static QSharedPointer<AlterationPayload> autoCastFromNetwork(QVariantHash &hash) {
            
            auto lowCast = AlterationPayload(hash);
            
            switch(lowCast.type()) {
                
                case PayloadAlteration::BulkMetadataChanged:
                    return QSharedPointer<BulkMetadataChangedPayload>(new BulkMetadataChangedPayload(hash));
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

                case PayloadAlteration::Duplicated:
                    return QSharedPointer<DuplicatedPayload>(new DuplicatedPayload(hash));
                break;

                case PayloadAlteration::OwnerChanged:
                    return QSharedPointer<OwnerChangedPayload>(new OwnerChangedPayload(hash));
                break;

                case PayloadAlteration::MetadataChanged:
                    return QSharedPointer<MetadataChangedPayload>(new MetadataChangedPayload(hash));
                break;

                case PayloadAlteration::Redone:
                    return QSharedPointer<RedonePayload>(new RedonePayload(hash));
                break;

                case PayloadAlteration::Undone:
                    return QSharedPointer<UndonePayload>(new UndonePayload(hash));
                break;
            }

        }
};