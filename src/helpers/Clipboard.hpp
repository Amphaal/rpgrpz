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

        static const QVector<RPZAtomId> get() {
            QMutexLocker m(&_m_clipboard);
            return _copyClipboard;
        }

        static void set(const QVector<RPZAtomId> &ids) {
            QMutexLocker m(&_m_clipboard);
            _copyClipboard = ids;
        }
        

    private:
        static inline QMutex _m_clipboard;
        static inline QVector<RPZAtomId> _copyClipboard;
};