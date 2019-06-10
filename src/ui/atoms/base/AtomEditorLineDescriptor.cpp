#include "AtomEditorLineDescriptor.h"

AtomEditorLineDescriptor::AtomEditorLineDescriptor(const QString &description, const QString &suffix) : 
    _suffix(suffix),
    _valLbl(new QLabel(this)) {
    
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    
    auto hLayout = new QHBoxLayout;
    this->setLayout(hLayout);
    
    hLayout->addWidget(new QLabel(description));
    hLayout->addStretch(0);
    hLayout->addWidget(this->_valLbl);
}

void AtomEditorLineDescriptor::updateValue(double value) {
    this->_valLbl->setText(QString::number(value) + this->_suffix);
}

void AtomEditorLineDescriptor::cannotDisplayValue() {
    this->_valLbl->setText("val. mult.");
}