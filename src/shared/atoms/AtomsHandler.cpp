#include "AtomsHandler.h"

AtomsHandler::AtomsHandler(const AlterationPayload::Source &boundSource) : _source(boundSource) { }

AlterationPayload::Source AtomsHandler::source() {
    return this->_source;
}

QFuture<void> AtomsHandler::handleAlterationRequest(AlterationPayload &payload, bool autoPropagate) {
    
    //trace
    auto self = AlterationPayload::SourceAsStr[this->source()];
    auto source = AlterationPayload::SourceAsStr[payload.source()];
    auto alterationType = PayloadAlterationAsString[payload.type()];
    qDebug() << "Alteration :" << self << "received" << alterationType << "from" << source;

    // handling promise
    auto alterationHandled = QtConcurrent::run([=]() mutable {
        return this->_handlePayload(payload);
    });
    
    //if propagation required, wait for alteration to finish, then wait for propagation to finish before completing
    if(autoPropagate) {
        return AsyncFuture::observe(alterationHandled).subscribe([=]() mutable {
           return this->propagateAlterationPayload(payload);
        }).future();
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