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

#include <QMutexLocker>
#include <QMutex>
#include "src/shared/models/RPZSharedDocument.hpp"

class SharedDocHint {
    public:
        static RPZSharedDocument::NamesStore getNamesStore() {
            QMutexLocker m(&_mNS);
            return _namesStore;
        }

        static void updateNamesStore(const RPZSharedDocument::FileHash &hash, const RPZSharedDocument::DocumentName &docName) {
            QMutexLocker m(&_mNS);
            _namesStore.insert(hash, docName);
        }

        static void removeFromNamesStore(const RPZSharedDocument::FileHash &hash) {
            QMutexLocker m(&_mNS);
            _namesStore.remove(hash);
        }

        static bool containsHash(const RPZSharedDocument::FileHash &hash) {
            QMutexLocker m(&_mNS);
            return _namesStore.contains(hash);
        }

        static void insertIntoStore(const RPZSharedDocument &sd) {
            QMutexLocker m(&_mS);
            _store.insert(sd.documentFileHash(), sd);
        }

        static RPZSharedDocument getSharedDocument(const RPZSharedDocument::FileHash &hash) {
            QMutexLocker m(&_mS);
            return _store.value(hash);
        }

    private:
        static inline QMutex _mNS;
        static inline RPZSharedDocument::NamesStore _namesStore;
        
        static inline QMutex _mS;
        static inline RPZSharedDocument::Store _store;

};

