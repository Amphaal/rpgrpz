#pragma once

#include <QProgressBar>

#include "src/shared/models/character/RPZGauge.hpp"

class Gauge : public QProgressBar {
    public:
        Gauge(const RPZGauge &gauge = RPZGauge()) {

            //bar
            this->setMinimum(gauge.minGaugeValue());
            this->setMaximum(gauge.maxGaugeValue());
            this->setValue(gauge.gaugeValue());
            this->setFormat("%v/%m ");
            this->setToolTip(gauge.name());
            
            this->_applyColor(gauge.color());

        }
    
    private:
        static inline QString _styleTemplate = "QProgressBar {border: 1px solid grey; border-radius: 3px; text-align: right;} QProgressBar::chunk {background-color: %1;width: 20px;}";
        
        void _applyColor(const QColor &color) {
            
            //apply to bar
            auto formatted = _styleTemplate.arg(color.name());
            this->setStyleSheet(formatted);

        }
};