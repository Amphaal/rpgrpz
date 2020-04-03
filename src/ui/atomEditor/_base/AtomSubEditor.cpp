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

#include "AtomSubEditor.h"

AtomSubEditor::AtomSubEditor(const QList<RPZAtom::Parameter> &parameters, bool supportsBatchEditing) :
    _params(parameters),
    _descr(new AtomEditorLineDescriptor(parameters.first(), supportsBatchEditing)),
    _supportsBatchEditing(supportsBatchEditing) { 

    this->setVisible(false);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->_mainLayout = new QVBoxLayout;
    this->setLayout(this->_mainLayout);
    this->layout()->addWidget(this->_descr);

}

const QList<RPZAtom::Parameter> AtomSubEditor::params() {
    return this->_params;
}

bool AtomSubEditor::mustShowBrushPenWidth(const QVariant &brushTypeDefaultValue) {
    auto cast = (RPZAtom::BrushType)brushTypeDefaultValue.toInt();
    return cast == RPZAtom::BrushType::RoundBrush;
}


void AtomSubEditor::_handleVisibilityOnLoad(const RPZAtom::Updates &defaultValues) {
    
    //default behavior if not a penWidth param
    if(!this->_params.contains(RPZAtom::Parameter::BrushPenWidth)) {
        this->setVisible(true);
        return;
    }

    //check
    auto brushStyleVal = defaultValues.value(RPZAtom::Parameter::BrushStyle);
    if(brushStyleVal.isNull()) {
        return;
    }

    //visibility
    auto mustShow = mustShowBrushPenWidth(brushStyleVal);
    this->setVisible(mustShow);
}


void AtomSubEditor::loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {
    
    this->setEnabled(true);

    //handle visibility
    this->_handleVisibilityOnLoad(defaultValues);

    auto hasEmptyValue = false;
    for(const auto &param : this->_params) {
        auto associatedValue = defaultValues.value(param);
        if(associatedValue.isNull()) hasEmptyValue = true;
    }
    
    //replace descr if has empty
    if(hasEmptyValue) {
        this->_descr->cannotDisplayValue();
        if(!this->_supportsBatchEditing) this->setEnabled(false);
    }

}
