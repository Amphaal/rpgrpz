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
// different license and copyright still refer to this GNU General Public License.

#include "RandomColor.h"

QColor RandomColor::getRandomKellyColor(const RandomColor::Context &context) {
    
    //fill if empty
    if(_availableColorsLeft.value(context).isEmpty()) {
        _availableColorsLeft.insert(context, _kellyColors);
    }

    auto &inC = _availableColorsLeft[context];

    //if single left, return it
    if(inC.count() == 1) return inC.takeFirst();
    
    //find index
    auto rPicked = QRandomGenerator::global()->bounded(0, inC.count() - 1);

    //return color
    return inC.takeAt(rPicked);

}

QList<QColor> RandomColor::generateColors(const RandomColor::Context &context, int howMany) {
    QList<QColor> list;

    for(int i = 0; i < howMany; i++) {
        auto color = getRandomKellyColor(context);
        list.append(color);
    }   

    return list;
} 