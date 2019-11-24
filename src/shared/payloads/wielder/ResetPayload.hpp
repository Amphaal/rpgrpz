#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class ResetPayload : public AtomsWielderPayload {
    public:
        ResetPayload() {}
        explicit ResetPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        ResetPayload(const MapDatabase &map) : AtomsWielderPayload(map) {
            this->_setMapParams(map.mapParams());
        }
        
        friend QDebug operator<<(QDebug debug, const ResetPayload &c) {
            QDebugStateSaver saver(debug);
            debug.nospace() << c.type() << ", count:" << c.atoms().count();
            return debug;
        }
    
        const RPZMapParameters mapParameters() const {
            return RPZMapParameters(this->value("mParams").toHash());
        }

        void setMapParams(const RPZMapParameters &mapParams) {
            this->insert("fromMPUpdate", true);
            this->_setMapParams(mapParams);
        }

        bool isFromMapParametersUpdate() const {
            return this->value("fromMPUpdate").toBool();
        }

    private:
        void _setMapParams(const RPZMapParameters &mapParams) {
            this->insert("mParams", mapParams);
        }
};

Q_DECLARE_METATYPE(ResetPayload)