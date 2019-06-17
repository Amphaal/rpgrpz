#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include <QLabel>
#include <QSpinBox>

class BrushToolWidthEditor : public QWidget {
    public:
        BrushToolWidthEditor() : _spin(new QSpinBox) {
            
            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            auto layout = new QHBoxLayout;
            this->setLayout(layout);
            
            auto descr = new QLabel("Taille du tampon :");

            this->_spin->setMinimum(1);
            this->_spin->setMaximum(50);
            
            layout->addWidget(descr, 1);
            layout->addWidget(this->_spin);
        }

        QSpinBox* spin() {
            return this->_spin;
        }

    private:
        QSpinBox* _spin = nullptr;
};