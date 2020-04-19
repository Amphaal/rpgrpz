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

#pragma once

#include <QList>
#include <QColor>
#include <QRandomGenerator>

#include <QDebug>

class RandomColor {
 public:
    enum Context {
        PlayerColor,
        LoaderColor
    };

    static QColor getRandomKellyColor(const RandomColor::Context &context);

    static QList<QColor> generateColors(
        const RandomColor::Context &context,
        int howMany = 22);

 private:
    static inline const QList<QColor> _kellyColors {
        "#FFB300", "#803E75", "#FF6800", "#A6BDD7", "#C10020", "#CEA262", "#817066",  // OK
        "#007D34",  "#F6768E",  "#00538A", "#FF7A5C", "#53377A", "#FF8E00", "#B32851", "#F4C800", "#7F180D", "#93AA00", "#593315", "#F13A13", "#232C16"  // No good for defective color vision
    };

    static inline QHash<RandomColor::Context, QList<QColor>> _availableColorsLeft;
};
