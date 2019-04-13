#include "AudioStreamController.h"

AudioStreamController::AudioStreamController(QWidget * parent) : QGroupBox(parent) {
    this->setEnabled(false);
    this->setTitle("Liste de lecture");
    this->setAlignment(Qt::AlignHCenter);
};
