#pragma once

#include "src/shared/database/AssetsDatabase.h"
#include "src/shared/models/RPZAtom.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QGLWidget>

#include <QPropertyAnimation>

class MapViewInteractibleDescriptor : public QWidget {

    Q_OBJECT

    private:
        QLabel* _descrLbl = nullptr;
        QPropertyAnimation* _anim = nullptr;

        const QRect _hiddenGeometry() const {
            auto s = sizeHint();
            return QRect(QPoint{ -s.width(), 1 }, s);
        }

    public:
        MapViewInteractibleDescriptor() : _descrLbl(new QLabel), _anim(new QPropertyAnimation(this, "geometry")) {
            
            this->_anim->setEasingCurve(QEasingCurve::InCubic);
            this->_anim->setDuration(250);

            this->setAutoFillBackground(true);
            QPalette pal = palette();
            pal.setColor(QPalette::Window, Qt::white);
            this->setPalette(pal);

            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            
            this->setLayout(new QHBoxLayout);
            this->layout()->addWidget(this->_descrLbl);

            //define geometry
            this->setGeometry(this->_hiddenGeometry());

        }

    public slots:
        void updateFromAtom(const RPZAtom &atom) {  

            this->_anim->stop();

            if(atom.isEmpty()) {

                this->_anim->setStartValue(this->geometry());
                this->_anim->setEndValue(this->_hiddenGeometry());

            } 
            
            else {

                this->_descrLbl->setText(atom.toString());
                auto sh = this->sizeHint();

                this->_anim->setStartValue(this->geometry());
                this->_anim->setEndValue(QRect({1, 1}, sh));


            }
            
            this->_anim->start();

        }

};