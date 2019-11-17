#pragma once

#include "src/shared/database/AssetsDatabase.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/sheets/components/GaugeWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QGLWidget>

#include <QPropertyAnimation>

class MapViewInteractibleDescriptor : public QWidget {

    Q_OBJECT

    private:
        GaugeWidget* _gaugeW = nullptr;
        QLabel* _descrLbl = nullptr;
        QLabel* _portraitLbl = nullptr;
        QPropertyAnimation* _anim = nullptr;

        const QRect _hiddenGeometry() const {
            auto s = sizeHint();
            return QRect(QPoint{ -s.width(), 1 }, s);
        }

        void _updateData(const RPZAtom &atom) {
            
            //update text descr
            this->_descrLbl->setText(atom.toString());
            
            auto isNPC = atom.type() == RPZAtom::Type::NPC;

            // update portrait
            QPixmap toSet;
            if(isNPC) {
                auto asset = AssetsDatabase::get()->asset(atom.assetHash());
                if(asset) toSet = QPixmap(asset->filepath());
                else toSet = QPixmap(QStringLiteral(u":/assets/default.jpg"));
            }
            this->_portraitLbl->setPixmap(toSet);
            this->_portraitLbl->setVisible(isNPC); 
            
            //update gauge
            auto mustDisplayGauge = false;
            if(isNPC) {
                auto gaugeValues = atom.NPCGauge();
                mustDisplayGauge = gaugeValues.current > -1;
                if(mustDisplayGauge) this->_gaugeW->updateValues(gaugeValues);
            }

            this->_gaugeW->setVisible(mustDisplayGauge);

        }

    public:
        MapViewInteractibleDescriptor() : _descrLbl(new QLabel), _portraitLbl(new QLabel), _anim(new QPropertyAnimation(this, "geometry")) {
            
            this->_gaugeW = new GaugeWidget({}, QObject::tr("Health"), Qt::red);
            this->_portraitLbl->setMinimumSize(1, 1);
            this->_portraitLbl->setMaximumSize(RPZCharacter::defaultPortraitSize / 2);

            this->_portraitLbl->setVisible(false);
            this->_gaugeW->setVisible(false);

            this->_anim->setEasingCurve(QEasingCurve::InCubic);
            this->_anim->setDuration(250);

            this->setAutoFillBackground(true);
            QPalette pal = palette();
            pal.setColor(QPalette::Window, Qt::white);
            this->setPalette(pal);

            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            
            auto layout = new QHBoxLayout;
            this->setLayout(layout);
            layout->addWidget(this->_portraitLbl, 0, Qt::AlignTop);
            layout->addWidget(this->_descrLbl, 0, Qt::AlignTop);
            layout->addWidget(this->_gaugeW, 0, Qt::AlignTop);

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

                this->_updateData(atom);

                this->_anim->setStartValue(this->geometry());
                this->_anim->setEndValue(QRect({1, 1}, this->sizeHint()));

            }
            
            this->_anim->start();

        }

};