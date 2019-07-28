#pragma once

#include <QSet>
#include "src/shared/payloads/Payloads.h"

class AtomAlterationAcknoledger {
    
    public:
        AtomAlterationAcknoledger() {
            _registeredAcknoledgers.insert(this);
        }
        ~AtomAlterationAcknoledger() {
            _registeredAcknoledgers.remove(this);
        }

        virtual void propagateAlteration(AlterationPayload &payload) {
            for(auto ack : _registeredAcknoledgers) {
                if(ack == this) continue; //do not self propagate
                ack->handleAlterationRequest(payload);
            }
        }

        virtual void handleAlterationRequest(AlterationPayload &payload) = 0;
    
    private:
        static inline QSet<AtomAlterationAcknoledger*> _registeredAcknoledgers;
};