#include "AudioStreamNotifier.h"

AudioStreamNotifier::AudioStreamNotifier(QWidget * parent) : QGroupBox(parent) {
    this->setTitle("Musique actuellement jouée");
    this->setAlignment(Qt::AlignHCenter);
    this->setEnabled(false);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    this->setLayout(new QHBoxLayout);
    this->layout()->addWidget(new QLabel("Pas de musique jouée"));
}