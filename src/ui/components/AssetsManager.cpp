#include "AssetsManager.h"

AssetsManager::AssetsManager(QWidget * parent) : QGroupBox(parent) {
    this->setTitle("Boite à Jouets");
    this->setMinimumWidth(200);
    this->setAlignment(Qt::AlignHCenter);
}