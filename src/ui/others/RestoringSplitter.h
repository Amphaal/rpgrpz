#pragma once

#include <QSplitter>
#include <QWidget>

#include "src/helpers/_appContext.h"

class RestoringSplitter : public QSplitter {
    
    public:
        RestoringSplitter(const QString &id, QWidget * parent = nullptr);

        void restore();

    private:
        const QString _id;
};
