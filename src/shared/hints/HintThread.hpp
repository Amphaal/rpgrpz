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

#include <QThread>

#include "src/shared/hints/MapHint.h"

class HintThread {
 public:
    static void init() {
        _hint = new MapHint;

        _hint->moveToThread(new QThread);
        _hint->thread()->setObjectName(QStringLiteral(u"MapThread"));

        QObject::connect(
            QApplication::instance()->thread(), &QThread::finished,
            [=]() {
                _hint->thread()->quit();
                _hint->deleteLater();
                _hint = nullptr;
        });

        _hint->thread()->start();
    }

    static MapHint* hint() {
        Q_ASSERT(_hint);
        return _hint;
    }

 private:
    static inline MapHint* _hint = nullptr;
};
