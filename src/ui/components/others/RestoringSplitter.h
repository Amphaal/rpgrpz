#pragma once

#include <QSplitter>
#include <QSettings>
#include <QWidget>

class RestoringSplitter : public QSplitter {
    
    public:
        RestoringSplitter(QString id, QWidget * parent = nullptr);
        void loadState();

    private:
        QSettings _intSettings;
        QString _id;
};
