#include "RPZStatusLabel.h"

RPZStatusLabel::RPZStatusLabel(QString descriptor, QWidget* parent) : QWidget(parent), 
    _descriptorLbl(new QLabel(descriptor + ":")), 
    _dataLbl(new QLabel),
    _loaderLbl(new QLabel) {
    
    if(!_loader) {

    }

    this->setLayout(new QHBoxLayout);
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->addWidget(this->_descriptorLbl);
    this->layout()->addWidget(this->_dataLbl);
    
    //default state
    this->updateState("<En attente...>", SL_Waiting);
}

void RPZStatusLabel::updateState(const QString &stateText, SLState state) {
    
    auto mustWait = (state == SL_Processing);
    this->setWaiting(mustWait);

    this->_dataLbl->setText(stateText);
};

void RPZStatusLabel::setWaiting(bool waiting) {
    
    //unset waiting
    if(!waiting) {

        if(this->_loader) {
            delete this->_loader;
            this->_loader = nullptr;
            delete this->_loaderLbl;
            this->_loaderLbl = nullptr;
        }

        return;
    }

    else {

        //if loader already displayed, do nothing
        if(this->_loader) return;

        //add load spinner
        this->_loader = new QMovie(":/icons/app/loader.gif");
        this->_loaderLbl = new QLabel();
        this->_loaderLbl->setMovie(this->_loader);
        this->_loader->start();
        ((QHBoxLayout*)this->layout())->insertWidget(0, this->_loaderLbl);

    }

}

QLabel* RPZStatusLabel::dataLabel() {
    return this->_dataLbl;
}