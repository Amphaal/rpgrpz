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

        static inline QSize _defaultPortraitSize = RPZCharacter::defaultPortraitSize / 1.5;

        GaugeWidget* _gaugeW = nullptr;
        QLabel* _descrLbl = nullptr;
        QLabel* _portraitLbl = nullptr;
        QLabel* _attitudeLbl = nullptr;
        QPropertyAnimation* _anim = nullptr;

        const QRect _hiddenGeometry() const {
            auto s = sizeHint();
            return QRect(QPoint{ -s.width(), 1 }, s);
        }

        void _updateData(const RPZAtom &atom) {
            
            auto isNPC = atom.type() == RPZAtom::Type::NPC;

            //update text descr
            this->_descrLbl->setText(atom.toString());
            
            //update attitude 
            if(isNPC) this->_attitudeLbl->setPixmap(QPixmap(atom.descriptiveIconPath()));
            else this->_attitudeLbl->setPixmap(QPixmap());
            this->_attitudeLbl->setVisible(isNPC);

            // update portrait
            QPixmap toSet;
            if(isNPC) {

                auto asset = AssetsDatabase::get()->asset(atom.assetHash());

                if(asset) toSet = QPixmap(asset->filepath());
                else toSet = QPixmap(QStringLiteral(u":/assets/default.jpg"));

                toSet = toSet.scaledToHeight(_defaultPortraitSize.height());

            }
            this->_portraitLbl->setPixmap(toSet);
            this->_portraitLbl->setVisible(isNPC); 
            
            //update gauge
            auto mustDisplayGauge = false;
            if(isNPC) {
                auto gaugeValues = atom.NPCGauge();
                mustDisplayGauge = gaugeValues.current != 0 || gaugeValues.min != 0 || gaugeValues.max != 0;
                if(mustDisplayGauge) this->_gaugeW->updateValues(gaugeValues);
            }

            this->_gaugeW->setVisible(mustDisplayGauge);

        }

    public:
        MapViewInteractibleDescriptor(QWidget *parent = nullptr) : QWidget(parent),
        _descrLbl(new QLabel), 
        _portraitLbl(new QLabel), 
        _attitudeLbl(new QLabel), 
        _anim(new QPropertyAnimation(this, "geometry")) {
            
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

            this->_attitudeLbl->setVisible(false);

            this->_portraitLbl->setFixedHeight(_defaultPortraitSize.height());
            this->_portraitLbl->setVisible(false);
            this->_portraitLbl->setStyleSheet("border: 1px solid black; background-color: white;");

            this->_gaugeW = new GaugeWidget({}, QObject::tr("Health"), "#ef4e4e");
            this->_gaugeW->setVisible(false);

            this->_anim->setEasingCurve(QEasingCurve::InCubic);
            this->_anim->setDuration(250);

            auto ssLayout = new QHBoxLayout;
            ssLayout->addWidget(this->_attitudeLbl);
            ssLayout->addWidget(this->_descrLbl, 1, Qt::AlignLeft);

            auto sLayout = new QVBoxLayout;
            sLayout->addLayout(ssLayout);
            sLayout->addWidget(this->_gaugeW);
            sLayout->addStretch(1);

            auto layout = new QHBoxLayout;
            layout->addWidget(this->_portraitLbl, 0, Qt::AlignTop);
            layout->addLayout(sLayout);
            this->setLayout(layout);

            //define geometry
            this->setGeometry(this->_hiddenGeometry());

        }

    public slots:
        void updateFromAtom(const RPZAtom &atom) {  

            this->_anim->stop();

            if(atom.isEmpty() || atom.type() != RPZAtom::Type::NPC) {

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

    protected:
        void paintEvent(QPaintEvent* event) {
            
            QPainter customPainter(this);
            
            auto rect = this->rect();
            customPainter.fillRect(rect, "#fcfcfc");

            rect.moveTopLeft({-1,-1});
            customPainter.setPen(QPen("#c0c0c0"));
            customPainter.drawRect(rect);

        }

};