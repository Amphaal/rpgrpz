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

#include "RestoringSplitter.h"

RestoringSplitter::RestoringSplitter(const QString &id, QWidget * parent) : QSplitter(parent), _id(id) {
    QObject::connect(this, &QSplitter::splitterMoved, [&]() {
        AppContext::settings()->setValue(this->_id, this->saveState());
    });
}

void RestoringSplitter::restore() {
    this->restoreState(AppContext::settings()->value(this->_id).toByteArray());

    this->setOpaqueResize(false);
    this->setHandleWidth(7);
}
