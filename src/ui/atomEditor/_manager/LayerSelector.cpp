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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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