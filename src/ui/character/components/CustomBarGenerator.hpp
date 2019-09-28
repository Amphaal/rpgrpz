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

class CustomBarGenerator : public QWidget {
    public:
        CustomBarGenerator() : 
            _bar(new QProgressBar), 
            _barNameEdit(new QLineEdit), 
            _currentValSpin(new QSpinBox),
            _minBarValSpin(new QSpinBox),
            _maxBarValSpin(new QSpinBox),
            _colorPicker(new QPushButton),
            _visibleOnPortraitChk(new QCheckBox) {

                //bar
                this->_bar->setMinimum(_defaultMin);
                this->_bar->setMaximum(_defaultMax);
                this->_bar->setValue(_defaultValue);
                this->_bar->setFormat("%v/%m ");
                
                //bar name 
                this->_barNameEdit->setPlaceholderText(" Nom de la jauge");
                this->_barNameEdit->setMinimumWidth(80);

                //visible on portrait
                this->_visibleOnPortraitChk->setText("Visible sous le portrait");

                //current
                this->_currentValSpin->setToolTip("Valeur de la jauge");
                this->_currentValSpin->setValue(_defaultValue);
                
                //minimum
                this->_minBarValSpin->setToolTip("Minimum de la jauge");
                this->_minBarValSpin->setValue(_defaultMin);

                //maximum
                this->_maxBarValSpin->setToolTip("Maximum de la jauge");
                this->_maxBarValSpin->setValue(_defaultMax);

                //color picker
                QObject::connect(
                    this->_colorPicker, &QPushButton::pressed,
                    this, &CustomBarGenerator::_onColorPickerPushed
                );

                //editors layout
                auto topL = new QHBoxLayout;
                topL->addWidget(this->_colorPicker);
                topL->addWidget(this->_barNameEdit);
                topL->addWidget(this->_visibleOnPortraitChk);
                topL->addStretch(1);
                topL->addWidget(this->_currentValSpin);
                topL->addSpacing(10);
                topL->addWidget(this->_minBarValSpin);
                topL->addWidget(this->_maxBarValSpin);

                //layout
                auto l = new QVBoxLayout;
                l->setMargin(0);
                this->setLayout(l);
                l->addLayout(topL);
                l->addWidget(this->_bar);

                //apply color to elements
                this->_applyColor();
        }
    
    private:
        QProgressBar* _bar = nullptr;
        QLineEdit* _barNameEdit = nullptr;
        QSpinBox* _currentValSpin = nullptr;
        QSpinBox* _minBarValSpin = nullptr;
        QSpinBox* _maxBarValSpin = nullptr;
        QPushButton* _colorPicker = nullptr;
        QCheckBox* _visibleOnPortraitChk = nullptr;

        static inline int _defaultValue = 15;
        static inline int _defaultMin = 0;
        static inline int _defaultMax = 30;

        static inline QString _styleTemplate = "QProgressBar {border: 1px solid grey; border-radius: 3px; text-align: right;} QProgressBar::chunk {background-color: %1;width: 20px;}";
        static inline QColor _defaultColor = QColor("red");

        QColor _currentColor = _defaultColor;

        void _applyColor(const QColor &color = QColor()) {
            
            //define current color
            this->_currentColor = color.isValid() ? color: _defaultColor;
            auto toApply = this->_currentColor.name();
            
            //apply to bar
            auto formatted = _styleTemplate.arg(toApply);
            this->_bar->setStyleSheet(formatted);

            //apply to color picker
            this->_colorPicker->setStyleSheet("background-color: " + toApply);

        }

        void _onColorPickerPushed() {
            
            auto selectedColor = QColorDialog::getColor(
                this->_currentColor,
                this,
                "Couleur de la jauge"
            );

            this->_applyColor(selectedColor);

        }
};