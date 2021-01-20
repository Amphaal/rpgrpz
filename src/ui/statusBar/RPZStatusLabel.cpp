// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include "RPZStatusLabel.h"

RPZStatusLabel::RPZStatusLabel(const QString &description, QWidget* parent) : QWidget(parent),
    _descriptionLbl(new QLabel(description + ":")),
    _dataLbl(new QLabel),
    _loaderLbl(new QLabel) {

    this->setLayout(new QHBoxLayout);
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->addWidget(this->_descriptionLbl);
    this->layout()->addWidget(this->_dataLbl);

    // default state
    this->updateState(tr("<Pending...>"), Waiting);
}

void RPZStatusLabel::updateState(const QString &stateText, RPZStatusLabel::State state) {
    auto mustWait = (state == Processing);
    this->setWaiting(mustWait);

    this->_dataLbl->setText(stateText);
}

void RPZStatusLabel::setWaiting(bool waiting) {
    // unset waiting
    if (!waiting) {
        if (this->_loader) {
            delete this->_loader;
            this->_loader = nullptr;
            delete this->_loaderLbl;
            this->_loaderLbl = nullptr;
        }

        return;
    } else {
        // if loader already displayed, do nothing
        if (this->_loader) return;

        // add load spinner
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
