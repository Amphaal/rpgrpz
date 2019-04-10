#pragma once

#include <QFrame>
#include <QColor>

class ColorIndicator : public QFrame {
    public:
        ColorIndicator(const QColor &color) {
            this->setFrameShape(QFrame::Shape::Box); 
            this->setLineWidth(1); 
            this->setFixedSize(10, 10);
            this->setAutoFillBackground(true);
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

            auto pal = this->palette();
            pal.setColor(QPalette::Background, color);
            this->setPalette(pal);
        };
};