#pragma once

#include <QSlider>
#include <QKeyEvent>

class RPZCustomSlider : public QSlider {
    public:
        RPZCustomSlider(Qt::Orientation orientation, QWidget *parent = nullptr) : QSlider(orientation, parent) { };

    private:
        void keyPressEvent(QKeyEvent *ev) override {
            ev->ignore();
        }
};