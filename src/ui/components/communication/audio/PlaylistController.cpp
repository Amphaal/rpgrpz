#include "PlaylistController.h"

PlaylistController::PlaylistController(QWidget * parent) : QGroupBox(parent) {
    this->setEnabled(false);
    this->setTitle("Liste de lecture");
    this->setAlignment(Qt::AlignHCenter);
};


void PlaylistController::bindToRPZClient(RPZClient * cc) {
    ClientBindable::bindToRPZClient(cc);
    this->setEnabled(true);

    QObject::connect(
       cc, &JSONSocket::disconnected,
       [&]() {
           this->setEnabled(false);
       }
    );
}