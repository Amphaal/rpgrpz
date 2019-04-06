#pragma once

#include <QSplitter>
#include <QSettings>
#include <QWidget>

class RestoringSplitter : public QSplitter {
    
    public:
        RestoringSplitter(const QString &id, QWidget * parent = nullptr) : QSplitter(parent), _id(id) {   
            QObject::connect(this, &QSplitter::splitterMoved, [&]() {
                this->_intSettings.setValue(this->_id, this->saveState());
            });
        }

        void loadState(){
            this->restoreState(this->_intSettings.value(this->_id).toByteArray());
            this->setHandleWidth(7);
        }

    private:
        QSettings _intSettings;
        const QString _id;
};
