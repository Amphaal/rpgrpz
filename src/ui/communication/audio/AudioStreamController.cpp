#include "AudioStreamController.h"

AudioStreamController::AudioStreamController(QWidget * parent) : QGroupBox(parent), 
    _descr(new QLabel(this)),
    toolbar(new AudioStreamToolbar(this)) {

    this->setEnabled(false);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    
    this->setTitle("Musique actuellement jouée");
    this->setAlignment(Qt::AlignHCenter);
    this->updatePlayedMusic(NULL);

    this->setLayout(new QHBoxLayout);
    this->layout()->addWidget(this->_descr);
    this->layout()->addWidget(this->toolbar);
}

void AudioStreamController::updatePlayedMusic(const QString &musicName) {
    if(musicName.isNull()) this->_descr->setText("Pas de musique jouée");
    else this->_descr->setText(musicName);
}

void AudioStreamController::bindToRPZClient(RPZClient * cc) {
    
    ClientBindable::bindToRPZClient(cc);
    this->setEnabled(true);

    QObject::connect(
       cc, &JSONSocket::disconnected,
       [&]() {
           this->setEnabled(false);
       }
    );
}