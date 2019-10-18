#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>

#include "src/shared/models/character/RPZGauge.hpp"

class GaugeEditor : public QWidget {

    Q_OBJECT
    
    public:
        GaugeEditor(const RPZGauge &gauge) : 
            _barNameEdit(new QLineEdit), 
            _currentValSpin(new QSpinBox),
            _minBarValSpin(new QSpinBox),
            _maxBarValSpin(new QSpinBox),
            _colorPicker(new QPushButton),
            _visibleOnPortraitChk(new QCheckBox) {
                
                //bar name 
                this->_barNameEdit->setPlaceholderText(tr(" Gauge name"));
                this->_barNameEdit->setText(gauge.name());
                this->_barNameEdit->setMinimumWidth(80);

                //visible on portrait
                this->_visibleOnPortraitChk->setText(tr("Fav."));
                this->_visibleOnPortraitChk->setToolTip(tr("Visible under portrait ?"));
                this->_visibleOnPortraitChk->setChecked(gauge.isVisibleUnderPortrait());
                
                //minimum
                this->_minBarValSpin->setToolTip(tr("Gauge minimum"));
                this->_minBarValSpin->setMinimum(-9999);
                this->_minBarValSpin->setValue(gauge.minGaugeValue());
                QObject::connect(this->_minBarValSpin, qOverload<int>(&QSpinBox::valueChanged), this, &GaugeEditor::_applyMinMaxLimitsOnSpinBoxes);

                //maximum
                this->_maxBarValSpin->setToolTip(tr("Gauge maximum"));
                this->_maxBarValSpin->setMaximum(9999);
                this->_maxBarValSpin->setValue(gauge.maxGaugeValue());
                QObject::connect(this->_maxBarValSpin, qOverload<int>(&QSpinBox::valueChanged), this, &GaugeEditor::_applyMinMaxLimitsOnSpinBoxes);

                //current
                this->_currentValSpin->setToolTip(tr("Gauge value"));
                this->_currentValSpin->setValue(gauge.gaugeValue());
                QObject::connect(this->_currentValSpin, qOverload<int>(&QSpinBox::valueChanged), this, &GaugeEditor::_applyMinMaxLimitsOnSpinBoxes);

                //limits
                this->_applyMinMaxLimitsOnSpinBoxes();

                //color picker
                QObject::connect(
                    this->_colorPicker, &QPushButton::pressed,
                    this, &GaugeEditor::_onColorPickerPushed
                );
                this->_applyColor(gauge.color());

                //remove bar button
                auto removeBarBtn = new QPushButton;
                removeBarBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/remove.png")));
                removeBarBtn->setToolTip(tr("Delete gauge"));
                QObject::connect(
                    removeBarBtn, &QPushButton::pressed,
                    this, &GaugeEditor::_onDeleteButonPressed
                );

                //editors layout
                auto topL = new QHBoxLayout;
                topL->addWidget(removeBarBtn);
                topL->addWidget(this->_colorPicker);
                topL->addWidget(this->_barNameEdit);
                topL->addWidget(this->_visibleOnPortraitChk);
                topL->addStretch(1);
                topL->addWidget(this->_minBarValSpin);
                topL->addSpacing(10);
                topL->addWidget(this->_currentValSpin);
                topL->addWidget(new QLabel("/"));
                topL->addWidget(this->_maxBarValSpin);

                //layout
                auto l = new QVBoxLayout;
                l->setMargin(0);
                this->setLayout(l);
                l->addLayout(topL);

        }

        RPZGauge toGauge() {
            RPZGauge g;

            g.setMinGaugeValue(this->_minBarValSpin->value());
            g.setMaxGaugeValue(this->_maxBarValSpin->value());
            g.setGaugeValue(this->_currentValSpin->value());
            g.setColor(this->_currentColor);
            g.setName(this->_barNameEdit->text()); 
            g.setVisibleUnderPortrait(this->_visibleOnPortraitChk->isChecked());

            return g;
        }
    
    private:
        QLineEdit* _barNameEdit = nullptr;
        QSpinBox* _currentValSpin = nullptr;
        QSpinBox* _minBarValSpin = nullptr;
        QSpinBox* _maxBarValSpin = nullptr;
        QPushButton* _colorPicker = nullptr;
        QCheckBox* _visibleOnPortraitChk = nullptr;

        QColor _currentColor;

        void _applyColor(const QColor &color) {
            
            //define current color
            this->_currentColor = color;
            auto toApply = this->_currentColor.name();

            //apply to color picker
            this->_colorPicker->setStyleSheet("background-color: " + toApply);

        }

        void _onColorPickerPushed() {
            
            auto selectedColor = QColorDialog::getColor(
                this->_currentColor,
                this,
                tr("Gauge color")
            );
            if(!selectedColor.isValid()) return;

            this->_applyColor(selectedColor);

        }

        void _onDeleteButonPressed() {

            auto result = QMessageBox::warning(
                this, 
                tr("Gauge deletion"), 
                tr("Do you really want to delete this gauge ?"),
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            
            if(result != QMessageBox::Yes) return;
            this->deleteLater();

        }

        void _applyMinMaxLimitsOnSpinBoxes() {
            
            //min
            this->_minBarValSpin->setMaximum(this->_currentValSpin->value());

            //max
            this->_maxBarValSpin->setMinimum(this->_currentValSpin->value());

            //value
            this->_currentValSpin->setMinimum(this->_minBarValSpin->value());
            this->_currentValSpin->setMaximum(this->_maxBarValSpin->value());

        }
};