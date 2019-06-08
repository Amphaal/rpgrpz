#pragma once

#include "payloads/AddedPayload.hpp"
#include "payloads/FocusedPayload.hpp"
#include "payloads/LayerChangedPayload.hpp"
#include "payloads/MovedPayload.hpp"
#include "payloads/RemovedPayload.hpp"
#include "payloads/ResetPayload.hpp"
#include "payloads/SelectedPayload.hpp"
#include "payloads/TextChangedPayload.hpp"
#include "payloads/RotatedPayload.hpp"
#include "payloads/ScaledPayload.hpp"
#include "payloads/VisibilityPayload.hpp"
#include "payloads/LockingPayload.hpp"
#include "payloads/DuplicatedPayload.hpp"

class Payload {
    public:
        static AlterationPayload* autoCast(QVariantHash &payload) {
            auto type = (AlterationPayload::Alteration)payload["t"].toInt();

            switch(type) {
                case AlterationPayload::Alteration::LockChanged:
                    return new LockingPayload(payload);
                case AlterationPayload::Alteration::VisibilityChanged:
                    return new VisibilityPayload(payload);
                case AlterationPayload::Alteration::Added:                  
                    return new AddedPayload(payload);
                case AlterationPayload::Alteration::Focused:
                    return new FocusedPayload(payload);
                case AlterationPayload::Alteration::LayerChanged:
                    return new LayerChangedPayload(payload);
                case AlterationPayload::Alteration::Moved:
                    return new MovedPayload(payload);
                case AlterationPayload::Alteration::Removed:
                    return new RemovedPayload(payload);
                case AlterationPayload::Alteration::Reset:
                    return new ResetPayload(payload);
                case AlterationPayload::Alteration::Selected:
                    return new SelectedPayload(payload);
                case AlterationPayload::Alteration::TextChanged:
                    return new TextChangedPayload(payload);
                case AlterationPayload::Alteration::Rotated:
                    return new RotatedPayload(payload);
                case AlterationPayload::Alteration::Scaled:
                    return new ScaledPayload(payload);
                case AlterationPayload::Alteration::Duplicated:
                    return new DuplicatedPayload(payload);
            }

            return new AlterationPayload(payload);
        }
};