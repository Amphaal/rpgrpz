#pragma once

#include <QSplashScreen>
#include <QPixmap>

class AppLoader : public QSplashScreen {
    public:
        AppLoader::AppLoader() {
            QPixmap loaderIcon(":/icons/app/rpgrpz.png");
            this->setPixmap(loaderIcon);
            this->show();
        }
};