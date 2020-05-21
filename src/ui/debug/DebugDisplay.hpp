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

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class DebugDisplay : public QWidget {
 public:
    static void updatePoint(const QPointF &point) {
        _get()->_lblPoint->setText(
            _pointTemplate
                .arg(point.x())
                .arg(point.y())
        );
    }

    static void updateIndex(double x, double y) {
        _get()->_lblIndex->setText(
            _pointTemplate
                .arg(x)
                .arg(y)
        );
    }

    static void updateCorrect(bool correctX, bool correctY) {
        _get()->_lblCorrect->setText(
            _pointTemplate
                .arg(correctX)
                .arg(correctY)
        );
    }

 private:
    static inline DebugDisplay* _self = nullptr;
    static DebugDisplay* _get() {
        if(!_self) {
            _self = new DebugDisplay;
            _self->show();
        }
        return _self;
    }
    QLabel* _lblPoint = nullptr;
    QLabel* _lblIndex = nullptr;
    QLabel* _lblCorrect = nullptr;

    static inline QString _pointTemplate = "{ %1 : %2 }";

    DebugDisplay() : QWidget(nullptr, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint),
    _lblPoint(new QLabel(this)),
    _lblIndex(new QLabel(this)),
    _lblCorrect(new QLabel(this)) {
        this->setFixedSize({300, 100});

        this->setLayout(new QVBoxLayout);
        this->layout()->addWidget(_lblPoint);
        this->layout()->addWidget(_lblIndex);
        this->layout()->addWidget(_lblCorrect);
    }
};
