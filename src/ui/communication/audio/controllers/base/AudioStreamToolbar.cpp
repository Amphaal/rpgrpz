#include "AudioStreamToolbar.h"

AudioStreamToolbar::AudioStreamToolbar(QWidget* parent) : QWidget(parent), 
    _audio(new QSlider), 
    _volumeStr(new QLabel),
    _mute(new QToolButton) {
    
    //audio
    this->_audio->setOrientation(Qt::Orientation::Horizontal);
    this->_audio->setFixedWidth(100);
    this->_audio->setMinimum(0);
    this->_audio->setMaximum(100);

    auto savedAV = AppContext::settings()->audioVolume();
    this->_volumeStr->setMinimumWidth(30);
    this->_setAudioValLbl(savedAV);
    this->_audio->setValue(savedAV);

    QObject::connect(
        this->_audio, &QAbstractSlider::valueChanged,
        this, &AudioStreamToolbar::_onAudioChange
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
    this->layout()->addWidget(this->_volumeStr);

    //self
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

AudioStreamToolbar::~AudioStreamToolbar() {}

void AudioStreamToolbar::_onAudioChange(int newSliderVal) {
    this->_setAudioValLbl(newSliderVal);
    AppContext::settings()->setAudioVolume(newSliderVal);
    emit askForVolumeChange(newSliderVal); 
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

void AudioStreamToolbar::_setAudioValLbl(int sliderVal) {
    this->_volumeStr->setText(QString::number(sliderVal) + QString("%"));
}