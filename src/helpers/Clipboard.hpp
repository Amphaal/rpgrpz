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
        static inline QMutex _m_clipboard;
        static inline QList<RPZAtom::Id> _copyClipboard;
};