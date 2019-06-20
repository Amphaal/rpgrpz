#include "TestMainWindow.h"

TestMainWindow::TestMainWindow() {

    this->setCentralWidget(new AudioManager);

    //initial show
    this->resize(800, 600);
    this->show();

}
