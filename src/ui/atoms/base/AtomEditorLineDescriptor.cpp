#include "AtomEditorLineDescriptor.h"

AtomEditorLineDescriptor::AtomEditorLineDescriptor(const AtomParameter &paramType) : AtomEditorLineDescriptor(
    _ParamDescr[paramType], 
    _valSuffix[paramType], 
    _icons[paramType]
) { }

AtomEditorLineDescriptor::AtomEditorLineDescriptor(const QString &description, const QString &suffix, QString &iconPath) : 
    _suffix(suffix),
    _valLbl(new QLabel) {
    
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    
    auto hLayout = new QHBoxLayout;
    this->setLayout(hLayout);
    
    //icon
    if(!iconPath.isEmpty()) {
        auto iconLbl = new QLabel;
        QPixmap icon(iconPath);
        iconLbl->setPixmap(icon);
        hLayout->addWidget(iconLbl);
    }

    //description
    auto descrLbl = new QLabel(description);
    hLayout->addWidget(descrLbl);
   
    hLayout->addStretch(0);
    hLayout->addWidget(this->_valLbl);

}

void AtomEditorLineDescriptor::updateValue(double value) {
    this->_valLbl->setText(QString::number(value) + this->_suffix);
}

void AtomEditorLineDescriptor::cannotDisplayValue() {
    this->_valLbl->setText("val. mult.");
}