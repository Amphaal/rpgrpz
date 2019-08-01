#pragma once

#include <QSet>
#include <QQueue>

#include "src/_libs/asyncfuture.h"

#include "src/shared/payloads/Payloads.h"

class AtomAlterationAcknoledger {
    
    public:
        AtomAlterationAcknoledger() {
            _registeredAcknoledgers.insert(this);
        }
        ~AtomAlterationAcknoledger() {
            _registeredAcknoledgers.remove(this);
        }
    
    protected:
        virtual QFuture<void> handleAlterationRequest(AlterationPayload &payload, bool autoPropagate = true) = 0;
        virtual QFuture<void> propagateAlterationPayload(AlterationPayload &payload) {
            
            //wait for all propagations to be done
            auto all = AsyncFuture::combine();

            for(auto ack : _registeredAcknoledgers) {
                if(ack == this) continue; //do not self propagate
                all << ack->handleAlterationRequest(payload, false); //prevent another propagation
            }
            
            return all.future();
        }

        void queueAlteration(AlterationPayload &payload, bool autoPropagate = true) {
            
            //add to queue
            auto instruction = [=]() mutable {
                return this->handleAlterationRequest(payload, autoPropagate);
            };
            _queuedAlterations.enqueue(instruction);
            
            //if no dequeuing running, start dequeuing
            if(_queuedAlterations.count() > 0 && !_dequeuing) _emptyQueue();
        }
    
    private:
        static inline QSet<AtomAlterationAcknoledger*> _registeredAcknoledgers;
        static QQueue<std::function<QFuture<void>()>> _queuedAlterations;
        
        static inline bool _dequeuing = false;
        static void _emptyQueue() {

            _dequeuing = true;

            auto instr = _queuedAlterations.dequeue();

            AsyncFuture::observe(instr()).subscribe([=]() {
                
                //if queue is empty, stop
                if(!_queuedAlterations.count()) {
                    _dequeuing = false;
                    return;
                }
                
                //keep dequeuing
                return _emptyQueue();

            });
        };
};