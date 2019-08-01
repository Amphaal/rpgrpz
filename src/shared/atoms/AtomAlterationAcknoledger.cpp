#include "AtomAlterationAcknoledger.h"


AtomAlterationAcknoledger::AtomAlterationAcknoledger(const AlterationPayload::Source &source, bool autoRegisterAck) : _source(source) {
    if(autoRegisterAck) _registeredAcknoledgers.insert(this);
}

AtomAlterationAcknoledger::~AtomAlterationAcknoledger() {
    _registeredAcknoledgers.remove(this);
}

AlterationPayload::Source AtomAlterationAcknoledger::source() {
    return this->_source;
}

void AtomAlterationAcknoledger::queueAlteration(AlterationPayload &payload, bool autoPropagate) {
    
    //add to queue
    auto instruction = [=]() {
        
        auto cPayload = Payloads::autoCast(payload);
        
        auto exec = this->_handleAlterationRequest(*cPayload, autoPropagate);

        if(cPayload->type() == PA_Reset) _resetAck(exec);

        return exec;

    };

    _queuedAlterations.enqueue(instruction);
    
    //if no dequeuing running, start dequeuing
    if(_queuedAlterations.count() > 0 && !_dequeuing) _emptyQueue();

}

QFuture<void> AtomAlterationAcknoledger::propagateAlterationPayload(AlterationPayload &payload) {
    
    //wait for all propagations to be done
    auto all = AsyncFuture::combine();

    for(auto ack : _registeredAcknoledgers) {
        if(ack == this) continue; //do not self propagate
        all << ack->_handleAlterationRequest(payload, false); //prevent another propagation
    }
    
    return all.future();
}

void AtomAlterationAcknoledger::resetAlterationRequested(QFuture<void> &alterationRequest) {};

void AtomAlterationAcknoledger::_payloadTrace(AlterationPayload &payload) {
    auto self = AlterationPayload::SourceAsStr[this->source()];
    auto source = AlterationPayload::SourceAsStr[payload.source()];
    auto alterationType = PayloadAlterationAsString[payload.type()];
    qDebug() << "Alteration :" << self << "received" << alterationType << "from" << source;
}

QFuture<void> AtomAlterationAcknoledger::_emptyQueue() {

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

void AtomAlterationAcknoledger::_resetAck(QFuture<void> &resetPromise) {
    for(auto ack : _registeredAcknoledgers) {
        ack->resetAlterationRequested(resetPromise);
    }
}