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

#include "RandomColor.h"

QColor RandomColor::getRandomColor() {
    
    if(!_rand) {
        _rand = QRandomGenerator::global()->bounded(1.0);
    } else {
        _rand = fmod(_rand + _goldenRatio, 1);
    }

    return QColor::fromHsvF(_rand, .5, .95);
}

QList<QColor> RandomColor::generateColors(int howMany) {
    QList<QColor> list;

    for(int i = 0; i < howMany; i++) {
        auto color = getRandomColor();
        list.append(color);
    }   

    return list;
} 