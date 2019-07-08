#include "AudioProbeController.h"

AudioProbeController::AudioProbeController(QWidget * parent) : QGroupBox("Musique actuellement jouée", parent), 
    _descr(new QLabel(this)),
    _playIconLbl(new QLabel(this)),
    _playIcon(new QMovie(":/icons/app/audio/musicPlaying.gif")),
    toolbar(new VolumeToolbar(this)) {

    this->setEnabled(true);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    this->setAlignment(Qt::AlignHCenter);

    this->updatePlayedMusic(NULL);
    this->_playIcon->stop();
    this->_playIconLbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    this->_playIconLbl->setMovie(this->_playIcon);
    this->_playIconLbl->setVisible(false);

    this->setLayout(new QHBoxLayout);
    this->layout()->addWidget(this->_playIconLbl);
    this->layout()->addWidget(this->_descr);
    this->layout()->addWidget(this->toolbar);
}

void AudioProbeController::updatePlayedMusic(const QString &musicName) {
    if(musicName.isNull()) {
        this->_descr->setText("Pas de musique jouée");
        this->_playIcon->stop();
        this->_playIconLbl->setVisible(false);
        this->_descr->setEnabled(false);
    }
    else {
        this->_descr->setText(musicName);
        this->_playIcon->start();
        this->_playIconLbl->setVisible(true);
        this->_descr->setEnabled(true);
    }
}

void AudioProbeController::changeTrackState(bool isPlaying) {
    if(isPlaying) this->_playIcon->start();
    else this->_playIcon->stop();
}