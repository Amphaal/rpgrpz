#pragma once

#include <QSplashScreen>
#include <QPixmap>

class AppLoader : public QSplashScreen {
    public:
        AppLoader() {
            QPixmap loaderIcon(":/icons/app/app.png");
            this->setPixmap(loaderIcon);
            this->show();
        }
};