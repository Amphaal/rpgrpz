#include "LayerSelector.h"

LayerSelector::LayerSelector(QWidget *parent) : QWidget(parent), _spinbox(new QSpinBox(this)) {
    auto layout = new QHBoxLayout;
    this->setLayout(layout);
    layout->setContentsMargins(2,0,0,0);

    auto lbl = new QLabel(tr("Default Layer:"), this);
    layout->addWidget(lbl, 1);

    this->_spinbox->setValue(AppContext::settings()->defaultLayer());
    this->_spinbox->setMinimum(INT_MIN);
    layout->addWidget(this->_spinbox);           
}

LayerSelector::~LayerSelector() {
    if(this->_spinbox) {
        AppContext::settings()->setDefaultLayer(this->_spinbox->value());
    }
}

QSpinBox* LayerSelector::spinbox() {
    return this->_spinbox;
}