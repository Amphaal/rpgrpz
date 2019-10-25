#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class ResetPayload : public AtomsWielderPayload {
    public:
        ResetPayload() {}
        explicit ResetPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        ResetPayload(const MapDatabase &map) : AtomsWielderPayload(map) {}
        
        friend QDebug operator<<(QDebug debug, const ResetPayload &c) {
            QDebugStateSaver saver(debug);
            debug.nospace() << c.type() << ", count:" << c.atoms().count();
            return debug;
        }
};