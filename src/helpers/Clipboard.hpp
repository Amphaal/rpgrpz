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

#pragma once

#include <QMutex>
#include <QMutexLocker>

#include "src/shared/models/RPZAtom.h"

class Clipboard {
    public:
        static void clear() {
            QMutexLocker m(&_m_clipboard);
            _copyClipboard.clear();
        }

        static const QList<RPZAtom::Id> get() {
            QMutexLocker m(&_m_clipboard);
            return _copyClipboard;
        }

        static void set(const QList<RPZAtom::Id> &ids) {
            QMutexLocker m(&_m_clipboard);
            _copyClipboard = ids;
        }
        

    private:
        static inline QMutex _m_clipboard = QMutex();
        static inline QList<RPZAtom::Id> _copyClipboard;
};