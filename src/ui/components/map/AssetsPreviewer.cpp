#include "AssetsPreviewer.h"

AssetsPreviewer::AssetsPreviewer(QWidget * parent) : QGroupBox(parent) {
    this->setTitle("Options");
    this->setAlignment(Qt::AlignHCenter);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    this->setMinimumHeight(250);
}