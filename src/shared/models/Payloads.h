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
#include "payloads/RedonePayload.hpp"
#include "payloads/UndonePayload.hpp"

class Payload {
    public:
        static AlterationPayload* autoCast(const QVariantHash &payload) {
            auto type = (PayloadAlteration)payload["t"].toInt();

            switch(type) {
                case PayloadAlteration::Redone:
                    return new RedonePayload(payload);
                case PayloadAlteration::Undone:
                    return new UndonePayload(payload);
                case PayloadAlteration::LockChanged:
                    return new LockingPayload(payload);
                case PayloadAlteration::VisibilityChanged:
                    return new VisibilityPayload(payload);
                case PayloadAlteration::Added:                  
                    return new AddedPayload(payload);
                case PayloadAlteration::Focused:
                    return new FocusedPayload(payload);
                case PayloadAlteration::LayerChanged:
                    return new LayerChangedPayload(payload);
                case PayloadAlteration::Moved:
                    return new MovedPayload(payload);
                case PayloadAlteration::Removed:
                    return new RemovedPayload(payload);
                case PayloadAlteration::Reset:
                    return new ResetPayload(payload);
                case PayloadAlteration::Selected:
                    return new SelectedPayload(payload);
                case PayloadAlteration::TextChanged:
                    return new TextChangedPayload(payload);
                case PayloadAlteration::Rotated:
                    return new RotatedPayload(payload);
                case PayloadAlteration::Scaled:
                    return new ScaledPayload(payload);
                case PayloadAlteration::Duplicated:
                    return new DuplicatedPayload(payload);
            }

            return new AlterationPayload(payload);
        }
};