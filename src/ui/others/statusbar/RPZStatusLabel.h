#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#include <QString>
#include <QMovie>

class RPZStatusLabel : public QWidget {

    public:
        enum State {Finished = 0, Waiting, Processing, Error };

        RPZStatusLabel(QString descriptor, QWidget* parent = nullptr);

        void updateState(const QString &stateText, int state = Finished);
        void setWaiting(bool waiting);
        QLabel* dataLabel();

    private:
        QLabel* _descriptorLbl;
        QLabel* _dataLbl;
        QLabel* _loaderLbl = nullptr;
        QMovie* _loader = nullptr;
       
};