#pragma once

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
                }
            );

            _hint->thread()->start();

        };

        static MapHint* hint() {
            Q_ASSERT(_hint);
            return _hint;
        };

    private:
        static inline MapHint* _hint = nullptr;

};

