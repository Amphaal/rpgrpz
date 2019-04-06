#pragma once

#include <QSplashScreen>
#include <QPixmap>

#include "MainWindow.h"

class AppLoader : public QSplashScreen {

    public:
        AppLoader() : QSplashScreen(QPixmap(":/icons/app/rpgrpz.png")) {
            this->show();
        }
    
};