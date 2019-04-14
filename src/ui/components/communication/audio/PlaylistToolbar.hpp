#pragma once

#include <QTime>
#include <QString>

#include <QWidget>
#include <QHBoxLayout>

#include <QLabel>
#include <QSlider>
#include <QToolButton>

class PlaylistToolbar : public QWidget {
    
    Q_OBJECT

    public:
        enum Action { Rewind, Forward, Play, Pause };

        PlaylistToolbar(QWidget* parent = nullptr) : QWidget(parent), 
            _playBtn(new QToolButton), 
            _rewindBtn(new QToolButton),
            _forwardBtn(new QToolButton), 
            _trackStateSlider(new QSlider), 
            _trackPlayStateLbl(new QLabel) {
            
            //play / pause
            this->_playBtn->setCheckable(true);
            this->_onPlayButtonPressed();
            QObject::connect(
                this->_playBtn, &QAbstractButton::clicked,
                this, &PlaylistToolbar::_onPlayButtonPressed
            );
            
            //rewind
            this->_rewindBtn->setIcon(QIcon(":/icons/app/audio/rewind.png"));
            QObject::connect(
                this->_rewindBtn, &QAbstractButton::clicked,
                [&]() {
                    emit actionRequired(PlaylistToolbar::Action::Rewind); 
                }
            );

            //forward
            this->_forwardBtn->setIcon(QIcon(":/icons/app/audio/forward.png"));
            QObject::connect(
                this->_forwardBtn, &QAbstractButton::clicked,
                [&]() {
                    emit actionRequired(PlaylistToolbar::Action::Forward);
                }
            );

            //track state
            this->_trackStateSlider->setOrientation(Qt::Orientation::Horizontal);
            this->updateTrackState(-1);

            //layout
            this->setLayout(new QHBoxLayout);
            this->layout()->addWidget(this->_rewindBtn);
            this->layout()->addWidget(this->_playBtn);
            this->layout()->addWidget(this->_forwardBtn);
            this->layout()->addWidget(this->_trackStateSlider);
            this->layout()->addWidget(this->_trackPlayStateLbl);
        }
    

        void newTrack(int lengthInSeconds) {
            this->_trackStateSlider->setValue(0);
            this->_trackStateSlider->setMaximum(lengthInSeconds);

            this->_currentTrackEndFormated = this->_fromSecondsToTime(lengthInSeconds);

            this->updateTrackState(0);
        }

        void updateTrackState(int stateInSeconds) {

            QString current = stateInSeconds < 0 ? PlaylistToolbar::_defaultNoTime : this->_fromSecondsToTime(stateInSeconds);

            this->_trackPlayStateLbl->setText(
                this->_trackPlayStateTemplator
                    .arg(current)
                    .arg(this->_currentTrackEndFormated)
            );
        }

    signals:
        void actionRequired(const PlaylistToolbar::Action &action);

    private:
        QToolButton* _playBtn;
        QIcon _playIcon = QIcon(":/icons/app/audio/play.png");
        QIcon _pauseIcon = QIcon(":/icons/app/audio/pause.png");

        QToolButton* _rewindBtn;
        QToolButton* _forwardBtn;

        QSlider* _trackStateSlider;
        QLabel* _trackPlayStateLbl;
        static inline QString _defaultNoTime = "--"; 
        QString _trackPlayStateTemplator = QString(" %1 / %2 ");
        QString _currentTrackEndFormated = PlaylistToolbar::_defaultNoTime; 

        QString _fromSecondsToTime(int lengthInSeconds) {
            return QTime::fromMSecsSinceStartOfDay(lengthInSeconds * 1000).toString("hh:mm:ss");
        }

        void _onPlayButtonPressed() {
            if(this->_playBtn->isChecked()) {
                this->_playBtn->setIcon(this->_pauseIcon);
                emit actionRequired(PlaylistToolbar::Action::Pause);
            } else {
                this->_playBtn->setIcon(this->_playIcon);
                emit actionRequired(PlaylistToolbar::Action::Play);
            }
        }
};