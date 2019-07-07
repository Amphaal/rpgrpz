#include "PlaylistToolbar.h"

PlaylistToolbar::PlaylistToolbar(QWidget* parent) : QWidget(parent), 
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
    
    //on slider release
    QObject::connect(
        this->_trackStateSlider, &QSlider::sliderReleased,
        [&]() {
            this->_sliderDown = false;
            auto pos = this->_trackStateSlider->value();
            emit seeking(pos);
        }
    );
    QObject::connect(
        this->_trackStateSlider, &QSlider::sliderPressed,
        [&]() {this->_sliderDown = true;}
    );

    QObject::connect(
        this->_trackStateSlider, &QSlider::valueChanged,
        [&](int pos) {
            this->updateTrackState(pos);
            if(!this->_sliderDown) {
                emit seeking(pos);
            }
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

void PlaylistToolbar::updateTrackState(int stateInSeconds) {

    //conditionnal state widgets
    QString current = stateInSeconds < 0 ? PlaylistToolbar::_defaultNoTime : this->_fromSecondsToTime(stateInSeconds);
    this->_trackStateSlider->setEnabled(stateInSeconds > -1);
    
    //update slider value
    if(this->_trackStateSlider->isEnabled() && !this->_sliderDown) {
        this->_trackStateSlider->blockSignals(true); 
        this->_trackStateSlider->setValue(stateInSeconds);
        this->_trackStateSlider->blockSignals(false); 
    }

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

void PlaylistToolbar::endTrack() {
    this->_currentTrackEndFormated = PlaylistToolbar::_defaultNoTime;
    this->updateTrackState(-1);
}


void PlaylistToolbar::newTrack(int lengthInSeconds) {
    this->_trackStateSlider->setValue(0);
    this->_trackStateSlider->setMaximum(lengthInSeconds);

    this->_currentTrackEndFormated = this->_fromSecondsToTime(lengthInSeconds);

    this->updateTrackState(0);
    this->_setPlayButtonState(true);
}

QString PlaylistToolbar::_fromSecondsToTime(int lengthInSeconds) {
    return QTime::fromMSecsSinceStartOfDay(lengthInSeconds * 1000).toString("hh:mm:ss");
}

void PlaylistToolbar::_setPlayButtonState(bool isPlaying) {
    this->_playBtn->setChecked(isPlaying);
    this->_playBtn->setIcon(isPlaying ? this->_pauseIcon : this->_playIcon);
}

void PlaylistToolbar::_tooglePlayButtonState() {
    
    auto btnstate = this->_playBtn->isChecked();
    this->_setPlayButtonState(btnstate);
    
    if(btnstate) {
        emit actionRequired(PlaylistToolbar::Action::Play);
    } else {
        emit actionRequired(PlaylistToolbar::Action::Pause);
    }
}