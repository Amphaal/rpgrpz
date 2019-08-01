#pragma once

#include <functional>

#include <QSet>
#include <QQueue>

#include "src/_libs/asyncfuture.h"

#include "src/shared/payloads/Payloads.h"

class AtomAlterationAcknoledger {
    
    public:
        AtomAlterationAcknoledger(bool autoRegisterAck = true) {
            if(autoRegisterAck) _registeredAcknoledgers.insert(this);
        }

        ~AtomAlterationAcknoledger() {
            _registeredAcknoledgers.remove(this);
        }
        
        QFuture<void> queueAlteration(AlterationPayload &payload, bool autoPropagate = true) {
            
            //add to queue
            auto instruction = [=]() {
                auto cPayload = Payloads::autoCast(payload);
                return this->_handleAlterationRequest(*cPayload, autoPropagate);
            };
            _queuedAlterations.enqueue(instruction);
            
            //if no dequeuing running, start dequeuing
            if(_queuedAlterations.count() > 0 && !_dequeuing) return _emptyQueue();

            //else return immediately
            auto d = AsyncFuture::deferred<void>();
            d.complete();
            return d.future();
        }
    
    
    protected:
        virtual QFuture<void> _handleAlterationRequest(AlterationPayload &payload, bool autoPropagate = true) = 0;
        virtual QFuture<void> propagateAlterationPayload(AlterationPayload &payload) {
            
            //wait for all propagations to be done
            auto all = AsyncFuture::combine();

            for(auto ack : _registeredAcknoledgers) {
                if(ack == this) continue; //do not self propagate
                all << ack->_handleAlterationRequest(payload, false); //prevent another propagation
            }
            
            return all.future();
        }

    private:
        static inline QSet<AtomAlterationAcknoledger*> _registeredAcknoledgers;
        static inline QQueue<std::function<QFuture<void>()>> _queuedAlterations;
        
        static inline bool _dequeuing = false;
        static QFuture<void> _emptyQueue() {

            _dequeuing = true;

            auto instr = _queuedAlterations.dequeue();

            return AsyncFuture::observe(instr()).subscribe([=]()->QFuture<void>{
                
                //if queue is empty, stop
                if(!_queuedAlterations.count()) {
                    _dequeuing = false;
                    auto d = AsyncFuture::deferred<void>();
                    d.complete();
                    return d.future();
                }
                
                //keep dequeuing
                return _emptyQueue();

            }).future();
        };
};