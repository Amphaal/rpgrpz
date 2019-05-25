#pragma once

#include <QSplitter>
#include <QWidget>

#include "src/helpers/_appContext.h"

class RestoringSplitter : public QSplitter {
    
    public:
        RestoringSplitter(const QString &id, QWidget * parent = nullptr) : QSplitter(parent), _id(id) {   

            QObject::connect(this, &QSplitter::splitterMoved, [&]() {
                AppContext::settings()->setValue(this->_id, this->saveState());
            });
        }

        void loadState(){
            this->restoreState(AppContext::settings()->value(this->_id).toByteArray());
            this->setHandleWidth(7);
        }

    private:
        const QString _id;
};
