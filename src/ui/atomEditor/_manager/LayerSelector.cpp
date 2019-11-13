#include "LayerSelector.h"

LayerSelector::LayerSelector(QWidget *parent) : QWidget(parent) {
    
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setLayoutDirection(Qt::LeftToRight);
    
    auto layout = new QHBoxLayout;
    this->setLayout(layout);

    auto lbl = new QLabel(tr("Layer"), this);
    layout->addWidget(lbl);

    this->_spinbox = new QSpinBox(this);
    this->_spinbox->setValue(AppContext::settings()->defaultLayer());
    this->_spinbox->setMinimum(AppContext::MINIMUM_LAYER);
    this->_spinbox->setMaximum(AppContext::MAXIMUM_LAYER);
    layout->addWidget(this->_spinbox);

}

LayerSelector::~LayerSelector() {
    if(this->_spinbox) {
        auto val = this->_spinbox->value();
        AppContext::settings()->setDefaultLayer(val);
    }
}

QSpinBox* LayerSelector::spinbox() {
    return this->_spinbox;
}