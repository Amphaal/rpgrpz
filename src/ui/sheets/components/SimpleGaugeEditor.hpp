// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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

class SimpleGaugeEditor : public QWidget {
    Q_OBJECT

 public:
    void fillValues(const RPZGauge::MinimalistGauge &gauge) {
        // limits
        this->_applyMinMaxLimitsOnSpinBoxes(gauge);

        this->_currentValSpin->setValue(gauge.current);
        this->_minBarValSpin->setValue(gauge.min);
        this->_maxBarValSpin->setValue(gauge.max);
    }

    SimpleGaugeEditor() : _currentValSpin(new QSpinBox), _minBarValSpin(new QSpinBox), _maxBarValSpin(new QSpinBox) {

            // minimum
            this->_minBarValSpin->setToolTip(tr("Gauge minimum"));
            this->_minBarValSpin->setMinimum(-9999);
            QObject::connect(
                this->_minBarValSpin, qOverload<int>(&QSpinBox::valueChanged),
                this, &SimpleGaugeEditor::_applyDefaultMinMaxLimitsOnSpinBoxes
            );

            // maximum
            this->_maxBarValSpin->setToolTip(tr("Gauge maximum"));
            this->_maxBarValSpin->setMaximum(9999);
            QObject::connect(
                this->_maxBarValSpin, qOverload<int>(&QSpinBox::valueChanged),
                this, &SimpleGaugeEditor::_applyDefaultMinMaxLimitsOnSpinBoxes
            );

            // current
            this->_currentValSpin->setToolTip(tr("Gauge value"));
            QObject::connect(
                this->_currentValSpin, qOverload<int>(&QSpinBox::valueChanged),
                this, &SimpleGaugeEditor::_applyDefaultMinMaxLimitsOnSpinBoxes
            );

            // editors layout
            this->_layoutToInsertTo = new QHBoxLayout;
            this->_layoutToInsertTo->addWidget(this->_minBarValSpin);
            this->_layoutToInsertTo->addSpacing(10);
            this->_layoutToInsertTo->addWidget(this->_currentValSpin);
            this->_layoutToInsertTo->addWidget(new QLabel("/"));
            this->_layoutToInsertTo->addWidget(this->_maxBarValSpin);

            // layout
            auto l = new QVBoxLayout;
            l->setMargin(0);
            this->setLayout(l);
            l->addLayout(this->_layoutToInsertTo);
    }

    explicit SimpleGaugeEditor(const RPZGauge::MinimalistGauge &setup) : SimpleGaugeEditor() {
        this->fillValues(setup);
    }

    const RPZGauge::MinimalistGauge toMinimalist() const {
        RPZGauge::MinimalistGauge g;

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

    void _applyMinMaxLimitsOnSpinBoxes(const RPZGauge::MinimalistGauge &gauge) {
        // min
        this->_minBarValSpin->setMaximum(gauge.current);

        // max
        this->_maxBarValSpin->setMinimum(gauge.current);

        // value
        this->_currentValSpin->setMinimum(gauge.min);
        this->_currentValSpin->setMaximum(gauge.max);
    }

    void _applyDefaultMinMaxLimitsOnSpinBoxes() {
        this->_applyMinMaxLimitsOnSpinBoxes({
            this->_currentValSpin->value(),
            this->_minBarValSpin->value(),
            this->_maxBarValSpin->value()
        });
    }
};
