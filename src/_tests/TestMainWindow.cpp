#include "TestMainWindow.h"

TestMainWindow::TestMainWindow() {

    auto manager = new AudioManager;
    this->setCentralWidget(manager);

    //initial show
    this->resize(800, 600);
    this->show();

    //test url
    manager->_plCtrl->playlist->addYoutubeVideo("https://www.youtube.com/watch?v=-lBJ3ZYy_XA");
    manager->_plCtrl->playlist->addYoutubeVideo("https://www.youtube.com/watch?v=ugytTmQbfzI");

}
