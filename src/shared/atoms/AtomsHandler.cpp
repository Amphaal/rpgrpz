#include "AtomsHandler.h"

AtomsHandler::AtomsHandler(
        const AlterationPayload::Source &boundSource, 
        bool autoRegisterAck
    ) : AtomAlterationAcknoledger(boundSource, autoRegisterAck) { }

QFuture<void> AtomsHandler::_handleAlterationRequest(AlterationPayload &payload, bool autoPropagate) {
    
    //trace
    this->_payloadTrace(payload);

    // handling promise
    auto alterationHandled = QtConcurrent::run([=]() {
        auto cPayload = Payloads::autoCast(payload);
        return this->_handlePayload(*cPayload);
    });
    
    //if propagation required, waitAll (alteration and propagation) to finish before completing
    if(autoPropagate) {
        auto all = AsyncFuture::combine();
        all << alterationHandled;
        all << this->propagateAlterationPayload(payload);
        return all.future();
    }

    //else, just wait for alteration to finish
    return alterationHandled;
}

QFuture<void> AtomsHandler::propagateAlterationPayload(AlterationPayload &payload) {

    //if inner payload, apply own source for send
    auto source = payload.source();
    if(source == AlterationPayload::Source::Undefined) payload.changeSource(this->source()); 

    //propagate
    return AtomAlterationAcknoledger::propagateAlterationPayload(payload);
    
}