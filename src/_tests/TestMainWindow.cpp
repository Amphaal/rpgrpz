#include "TestMainWindow.h"

TestMainWindow::TestMainWindow() {

    auto manager = new AudioManager;
    this->setCentralWidget(manager);

    //initial show
    this->resize(800, 600);
    this->show();

    //test url
    manager->_plCtrl->playlist->addYoutubeVideo("https://www.youtube.com/watch?time_continue=211&v=6p03KD5IBrI");

}
