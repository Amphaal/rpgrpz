#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class ClientActivityIndicator : public QWidget {
    public:
        ClientActivityIndicator() {

            this->setVisible(false);
            this->setLayout(new QHBoxLayout);
            this->layout()->setContentsMargins(0, 0, 0, 0);
            this->layout()->setSpacing(2);

            _upGreyed = QPixmap(":/icons/app/connectivity/uploadG.png");;
            _downGreyed = QPixmap(":/icons/app/connectivity/downloadG.png");

            _down = QPixmap(":/icons/app/connectivity/download.png");
            _up = QPixmap(":/icons/app/connectivity/upload.png");

            _upLbl = new QLabel;
            _upLbl->setContentsMargins(0, 0, 0, 0);
            _upLbl->setPixmap(_upGreyed);

            _downLbl = new QLabel;
            _downLbl->setContentsMargins(0, 0, 0, 0);
            _downLbl->setPixmap(_downGreyed);

            this->layout()->addWidget(_upLbl);
            this->layout()->addWidget(_downLbl);


            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientSending,
                [=]() { _upLbl->setPixmap(_up); }
            );

            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientSent,
                [=]() { _upLbl->setPixmap(_upGreyed); }
            );

            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientReceiving,
                [=]() { _downLbl->setPixmap(_down); }
            );

            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientReceived,
                [=]() { _downLbl->setPixmap(_downGreyed); }
            );

        };
    
    private:
        QLabel* _upLbl;
        QLabel* _downLbl;

        QPixmap _upGreyed;
        QPixmap _downGreyed;

        QPixmap _up;
        QPixmap _down;
};