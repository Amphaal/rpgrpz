#include "AppLoader.h"

AppLoader::AppLoader() {
    QPixmap loaderIcon(":/icons/app/rpgrpz.png");
    this->setPixmap(loaderIcon);
    this->show();
}