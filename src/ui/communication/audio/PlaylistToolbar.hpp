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
            this->_setPlayButtonState(false);
            QObject::connect(
                this->_playBtn, &QAbstractButton::clicked,
                this, &PlaylistToolbar::_tooglePlayButtonState
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
            QObject::connect(
                this->_trackStateSlider, &QSlider::valueChanged,
                [&](int newVal) {
                    emit seeking(newVal);
                }
            );

            //layout
            this->setLayout(new QHBoxLayout);
            this->layout()->addWidget(this->_rewindBtn);
            this->layout()->addWidget(this->_playBtn);
            this->layout()->addWidget(this->_forwardBtn);
            this->layout()->addWidget(this->_trackStateSlider);
            this->layout()->addWidget(this->_trackPlayStateLbl);
        }
    
        void updateTrackState(int stateInSeconds) {

            //conditionnal state widgets
            QString current = stateInSeconds < 0 ? PlaylistToolbar::_defaultNoTime : this->_fromSecondsToTime(stateInSeconds);
            this->_trackStateSlider->setEnabled(stateInSeconds > -1);
            this->_playBtn->setEnabled(stateInSeconds > -1);
            this->_rewindBtn->setEnabled(stateInSeconds > -1);
            this->_forwardBtn->setEnabled(stateInSeconds > -1);

            //update txt val
            this->_trackPlayStateLbl->setText(
                this->_trackPlayStateTemplator
                    .arg(current)
                    .arg(this->_currentTrackEndFormated)
            );
        }


        void newTrack(int lengthInSeconds) {
            this->_trackStateSlider->setValue(0);
            this->_trackStateSlider->setMaximum(lengthInSeconds);

            this->_currentTrackEndFormated = this->_fromSecondsToTime(lengthInSeconds);

            this->updateTrackState(0);
            this->_setPlayButtonState(true);
        }

    signals:
        void actionRequired(const PlaylistToolbar::Action &action);
        void seeking(int pos);

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

        void _setPlayButtonState(bool isPlaying) {
            this->_playBtn->setChecked(isPlaying);
            this->_playBtn->setIcon(isPlaying ? this->_pauseIcon : this->_playIcon);
        }

        void _tooglePlayButtonState() {
            
            auto btnstate = this->_playBtn->isChecked();
            this->_setPlayButtonState(btnstate);
            
            if(btnstate) {
                emit actionRequired(PlaylistToolbar::Action::Pause);
            } else {
                emit actionRequired(PlaylistToolbar::Action::Play);
            }
        }
};