#pragma once

#include "payloads/AddedPayload.hpp"
#include "payloads/FocusedPayload.hpp"
#include "payloads/LayerChangedPayload.hpp"
#include "payloads/MovedPayload.hpp"
#include "payloads/RemovedPayload.hpp"
#include "payloads/ResetPayload.hpp"
#include "payloads/SelectedPayload.hpp"
#include "payloads/TextChangedPayload.hpp"

class Payload {
    public:
        static AlterationPayload* autoCast(QVariantHash &payload) {
            auto type = (AlterationPayload::Alteration)payload["t"].toInt();

            switch(type) {
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
            }

            return new AlterationPayload(payload);
        }
};