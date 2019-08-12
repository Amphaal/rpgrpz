#include "AlterationAcknoledger.h"

AlterationAcknoledger::AlterationAcknoledger(const AlterationPayload::Source &source, bool senderOnly) : _source(source) {
    if(!senderOnly) {
        _registeredAcknoledgers.insert(this);
    }
}

AlterationPayload::Source AlterationAcknoledger::source() {
    return this->_source;
}

void AlterationAcknoledger::_queueAlteration(AlterationPayload &payload, bool autoPropagate) {
    
    //add to queue
    auto instruction = [=]() {
        
        auto cPayload = Payloads::autoCast(payload);
        
        auto exec = this->_handleAlterationRequest(*cPayload, autoPropagate);

        return exec;

    };

    _queuedAlterations.enqueue(instruction);
    
    //if no dequeuing running, start dequeuing
    if(_queuedAlterations.count() > 0 && !_dequeuing) _emptyQueue();

}

QFuture<void> AlterationAcknoledger::propagateAlterationPayload(AlterationPayload &payload) {
    
    //wait for all propagations to be done
    auto all = AsyncFuture::combine();

    for(auto ack : _registeredAcknoledgers) {
        if(ack == this) continue; //do not self propagate
        all << ack->_handleAlterationRequest(payload, false); //prevent another propagation
    }
    
    return all.future();
}

void AlterationAcknoledger::_payloadTrace(AlterationPayload &payload) {
    auto self = AlterationPayload::SourceAsStr[this->source()];
    auto source = AlterationPayload::SourceAsStr[payload.source()];
    auto alterationType = PayloadAlterationAsString[payload.type()];
    qDebug() << "Alteration :" << self << "received" << alterationType << "from" << source;
}
