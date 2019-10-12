#include "AudioProbeController.h"

AudioProbeController::AudioProbeController(QWidget * parent) : QGroupBox(tr("Playing now"), parent), 
    _descr(new QLabel),
    _playIconLbl(new QLabel),
    _playIcon(new QMovie(":/icons/app/audio/musicPlaying.gif")),
    _posInTrackLbl(new QLabel),
    toolbar(new VolumeToolbar) {

    //bind animation to label
    this->setAlignment(Qt::AlignHCenter);
    this->_playIconLbl->setMovie(this->_playIcon);
    
    //layout
        auto h = new QHBoxLayout;
        h->setMargin(0);
        h->addWidget(this->_posInTrackLbl);
        h->addStretch(1);
        h->addWidget(this->toolbar, 3);

        auto v = new QVBoxLayout;
        v->setMargin(0);
        v->addWidget(this->_descr);
        v->addLayout(h);

        auto topMostLayout = new QHBoxLayout;
        this->setLayout(topMostLayout);
        topMostLayout->addWidget(this->_playIconLbl);
        topMostLayout->addLayout(v, 1);
    //END layout

    this->updatePlayedMusic(NULL);
    this->changeTrackPosition(-1);

}

void AudioProbeController::changeTrackPosition(int posInSecs) {
    auto text = posInSecs < 0 ? _defaultNoTime : DurationHelper::secondsToTrackDuration(posInSecs);
    this->_posInTrackLbl->setText(text);
}

void AudioProbeController::updatePlayedMusic(const QString &musicName) {
    
    auto musicIsPlaying = !musicName.isNull();
    
    if(!musicIsPlaying) {
        this->_descr->setText(tr("Nothing playing"));
        this->_playIcon->stop();
    }
    else {
        this->_descr->setText(musicName);
        this->_playIcon->start();
    }

    this->changeTrackPosition(-1);
    this->_posInTrackLbl->setVisible(musicIsPlaying);
    this->_playIconLbl->setVisible(musicIsPlaying);
    this->toolbar->setVisible(musicIsPlaying);
    
    this->_descr->setEnabled(musicIsPlaying);
    
}

void AudioProbeController::changeTrackState(bool isPlaying) {
    if(isPlaying) this->_playIcon->start();
    else this->_playIcon->stop();
}