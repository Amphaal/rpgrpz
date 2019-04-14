#include "AudioStreamController.h"

AudioStreamController::AudioStreamController(QWidget * parent) : QGroupBox(parent) {
    this->setEnabled(false);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    
    this->setTitle("Musique actuellement jouée");
    this->setAlignment(Qt::AlignHCenter);

    this->setLayout(new QHBoxLayout);
    this->layout()->addWidget(new QLabel("Pas de musique jouée"));
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