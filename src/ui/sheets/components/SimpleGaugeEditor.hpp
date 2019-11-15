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

class SimpleGaugeEditor : public QWidget {

    Q_OBJECT

    public:
        struct Gauge {
            int current; 
            int min; 
            int max;
        };
        
        void fillValues(const SimpleGaugeEditor::Gauge &gauge) {
            
            this->_currentValSpin->setValue(gauge.current);
            this->_minBarValSpin->setValue(gauge.min);
            this->_maxBarValSpin->setValue(gauge.max);
            
            //limits
            this->_applyMinMaxLimitsOnSpinBoxes();

        };

        SimpleGaugeEditor() : 
            _currentValSpin(new QSpinBox),
            _minBarValSpin(new QSpinBox),
            _maxBarValSpin(new QSpinBox) {
                
                //minimum
                this->_minBarValSpin->setToolTip(tr("Gauge minimum"));
                this->_minBarValSpin->setMinimum(-9999);
                QObject::connect(
                    this->_minBarValSpin, qOverload<int>(&QSpinBox::valueChanged), 
                    this, &SimpleGaugeEditor::_applyMinMaxLimitsOnSpinBoxes
                );

                //maximum
                this->_maxBarValSpin->setToolTip(tr("Gauge maximum"));
                this->_maxBarValSpin->setMaximum(9999);
                QObject::connect(
                    this->_maxBarValSpin, qOverload<int>(&QSpinBox::valueChanged), 
                    this, &SimpleGaugeEditor::_applyMinMaxLimitsOnSpinBoxes
                );

                //current
                this->_currentValSpin->setToolTip(tr("Gauge value"));
                QObject::connect(
                    this->_currentValSpin, qOverload<int>(&QSpinBox::valueChanged), 
                    this, &SimpleGaugeEditor::_applyMinMaxLimitsOnSpinBoxes
                );

                //editors layout
                this->_layoutToInsertTo = new QHBoxLayout;
                this->_layoutToInsertTo->addWidget(this->_minBarValSpin);
                this->_layoutToInsertTo->addSpacing(10);
                this->_layoutToInsertTo->addWidget(this->_currentValSpin);
                this->_layoutToInsertTo->addWidget(new QLabel("/"));
                this->_layoutToInsertTo->addWidget(this->_maxBarValSpin);

                //layout
                auto l = new QVBoxLayout;
                l->setMargin(0);
                this->setLayout(l);
                l->addLayout(this->_layoutToInsertTo);

        }

        SimpleGaugeEditor(const SimpleGaugeEditor::Gauge &setup) : SimpleGaugeEditor() {
            this->fillValues(setup);
        }

        const SimpleGaugeEditor::Gauge toSimpleGauge() const {
            
            SimpleGaugeEditor::Gauge g;

            g.min = this->_minBarValSpin->value();
            g.max = this->_maxBarValSpin->value();
            g.current = this->_currentValSpin->value();

            return g;
        }
    
    protected:
        QHBoxLayout* _layoutToInsertTo = nullptr;

    private:
        QSpinBox* _currentValSpin = nullptr;
        QSpinBox* _minBarValSpin = nullptr;
        QSpinBox* _maxBarValSpin = nullptr;

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