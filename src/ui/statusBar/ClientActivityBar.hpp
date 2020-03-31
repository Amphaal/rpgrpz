#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class ClientActivityIndicator : public QLabel {
    public:
        ClientActivityIndicator(const QPixmap &offState, const QPixmap &onState) : _off(offState), _on(onState) {

            this->setContentsMargins(0, 0, 0, 0);
            this->_setOff();

            QObject::connect(
                &_lingerTimer, &QTimer::timeout,
                [=](){
                    if(!_activityPending) _setOff();
                }
            );

        };

        void onActivityOn() {
            _activityPending = true;
            _lingerTimer.start(_indicatorLingerMs); 
            _setOn();
        }

        void onActivityOff() {
            _activityPending = false;
            if(!_lingerTimer.isActive()) _setOff();
        }
    
    private:
        QPixmap _on;
        QPixmap _off;

        static inline int _indicatorLingerMs = 330;

        QTimer _lingerTimer;
        bool _activityPending = false;

        void _setOff() {
            this->setPixmap(_off); 
        }

        void _setOn() {
            this->setPixmap(_on);    
        }
        
};

class ClientActivityBar : public QWidget {
    public:
        ClientActivityBar() {

            this->setVisible(false);
            this->setLayout(new QHBoxLayout);
            this->layout()->setContentsMargins(0, 0, 0, 0);
            this->layout()->setSpacing(2);

            //UPLOAD TRACKER
            _upLbl = new ClientActivityIndicator(
                QPixmap(":/icons/app/connectivity/uploadG.png"),
                QPixmap(":/icons/app/connectivity/upload.png")
            );
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientSending,
                _upLbl, &ClientActivityIndicator::onActivityOn
            );
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientSent,
                _upLbl, &ClientActivityIndicator::onActivityOff
            );
            this->layout()->addWidget(_upLbl);

            //DOWNLOAD TRACKER
            _downLbl = new ClientActivityIndicator(
                QPixmap(":/icons/app/connectivity/downloadG.png"),
                QPixmap(":/icons/app/connectivity/download.png")
            );
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientReceiving,
                _downLbl, &ClientActivityIndicator::onActivityOn
            );
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::clientReceived,
                _downLbl, &ClientActivityIndicator::onActivityOff
            );
            this->layout()->addWidget(_downLbl);

        };
    
    private:
        ClientActivityIndicator* _upLbl = nullptr;
        ClientActivityIndicator* _downLbl = nullptr;
        
};