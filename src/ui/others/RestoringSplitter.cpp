#include "RestoringSplitter.h"

RestoringSplitter::RestoringSplitter(const QString &id, QWidget * parent) : QSplitter(parent), _id(id) {   
    QObject::connect(this, &QSplitter::splitterMoved, [&]() {
        AppContext::settings()->setValue(this->_id, this->saveState());
    });
}

void RestoringSplitter::loadMap(){
    this->restoreState(AppContext::settings()->value(this->_id).toByteArray());
    
    this->setOpaqueResize(false);
    this->setHandleWidth(7);
}