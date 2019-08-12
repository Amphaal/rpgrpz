#include "AlterationHandler.h"

bool AlterationHandler::isDequeuing() {
    return _dequeuing;
}

QFuture<void> AlterationHandler::_emptyQueue() {

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