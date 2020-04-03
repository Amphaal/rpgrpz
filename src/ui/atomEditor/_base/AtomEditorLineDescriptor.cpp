// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#include "AtomEditorLineDescriptor.h"

AtomEditorLineDescriptor::AtomEditorLineDescriptor(const RPZAtom::Parameter &paramType, bool supportsBatchEditing) : AtomEditorLineDescriptor(
        _paramDescr.value(paramType), 
        _valSuffix.value(paramType), 
        _icons.value(paramType)
    ) { 
        this->_supportsBatchEditing = supportsBatchEditing;
    }

AtomEditorLineDescriptor::AtomEditorLineDescriptor(const QString &untranslatedDescription, const QString &suffix, const QString &iconPath) : 
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
    auto descrLbl = new QLabel;
    descrLbl->setText(
        tr(qUtf8Printable(untranslatedDescription))
    );
    hLayout->addWidget(descrLbl);
   
    hLayout->addStretch(0);
    hLayout->addWidget(this->_valLbl);

}

void AtomEditorLineDescriptor::updateValue(double value) {
    this->_valLbl->setText(QString::number(value) + this->_suffix);
}

void AtomEditorLineDescriptor::cannotDisplayValue() {
    this->_valLbl->setText(
       this->_supportsBatchEditing ? tr("mult. val.") : ""
    );
}