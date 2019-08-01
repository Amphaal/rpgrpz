#include "AtomsHandler.h"

AtomsHandler::AtomsHandler(const AlterationPayload::Source &boundSource, bool autoRegisterAck) : AtomAlterationAcknoledger(autoRegisterAck), _source(boundSource) { }

AlterationPayload::Source AtomsHandler::source() {
    return this->_source;
}

QFuture<void> AtomsHandler::_handleAlterationRequest(AlterationPayload &payload, bool autoPropagate) {
    
    //trace
    auto self = AlterationPayload::SourceAsStr[this->source()];
    auto source = AlterationPayload::SourceAsStr[payload.source()];
    auto alterationType = PayloadAlterationAsString[payload.type()];
    qDebug() << "Alteration :" << self << "received" << alterationType << "from" << source;

    // handling promise
    auto alterationHandled = QtConcurrent::run([=]() {
        auto cPayload = Payloads::autoCast(payload);
        return this->_handlePayload(*cPayload);
    });
    
    //if propagation required, wait for alteration and propagation to finish before completing
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
    if(source == AlterationPayload::Source::Undefined) payload.changeSource(this->_source); 

    //propagate
    return AtomAlterationAcknoledger::propagateAlterationPayload(payload);
    
}