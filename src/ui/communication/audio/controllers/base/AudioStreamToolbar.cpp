#include "AudioStreamToolbar.h"

AudioStreamToolbar::AudioStreamToolbar(QWidget* parent) : QWidget(parent), _audio(new QSlider), _mute(new QToolButton) {
    
    //audio
    this->_audio->setOrientation(Qt::Orientation::Horizontal);
    this->_audio->setMinimum(0);
    this->_audio->setMaximum(100);
    this->_audio->setValue(
        AppContext::settings()->audioVolume()
    );
    this->_audio->setFixedWidth(100);
    QObject::connect(
        this->_audio, &QAbstractSlider::valueChanged,
        [&](int value) {
            emit askForVolumeChange(value); 
        }
    );

    //mute
    this->_mute->setCheckable(true);
    this->_onMuteButtonClick();
    QObject::connect(
        this->_mute, &QAbstractButton::clicked,
        this, &AudioStreamToolbar::_onMuteButtonClick
    );

    //layout
    this->setLayout(new QHBoxLayout);
    this->layout()->addWidget(this->_mute);
    this->layout()->addWidget(this->_audio);

    //self
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

AudioStreamToolbar::~AudioStreamToolbar() {
    if(this->_audio) {
        AppContext::settings()->setAudioVolume(this->_audio->value());
    }
    
}

void AudioStreamToolbar::_onMuteButtonClick() {

    if(this->_mute->isChecked()) {
        this->_mute->setIcon(this->_mutedIcon);
        emit askForVolumeChange(0);
    } else {
        this->_mute->setIcon(this->_volumeIcon);
        emit askForVolumeChange(this->_audio->value());
    }

}