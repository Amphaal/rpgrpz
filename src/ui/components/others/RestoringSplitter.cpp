#include "RestoringSplitter.h"

RestoringSplitter::RestoringSplitter(QString id, QWidget * parent) : QSplitter(parent) {
    this->_id = id;
    
    QObject::connect(this, &QSplitter::splitterMoved, [&]() {
        this->_intSettings.setValue(this->_id, this->saveState());
    });
}

void RestoringSplitter::loadState() {
    this->restoreState(this->_intSettings.value(this->_id).toByteArray());
    this->setHandleWidth(7);
}