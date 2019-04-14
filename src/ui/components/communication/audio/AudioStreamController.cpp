#include "AudioStreamController.h"

AudioStreamController::AudioStreamController(QWidget * parent) : QGroupBox(parent), _toolbar(new AudioStreamToolbar(this)) {
    this->setEnabled(false);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    
    this->setTitle("Musique actuellement jouée");
    this->setAlignment(Qt::AlignHCenter);

    this->setLayout(new QHBoxLayout);
    this->layout()->addWidget(new QLabel("Pas de musique jouée"));
    this->layout()->addWidget(_toolbar);
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