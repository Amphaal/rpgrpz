#pragma once

#include <QProgressBar>

#include "src/shared/models/character/RPZGauge.hpp"

class GaugeWidget : public QProgressBar {
    public:
        GaugeWidget(const RPZGauge &gauge = RPZGauge()) {

            auto max = gauge.maxGaugeValue();
            auto val = gauge.gaugeValue();

            //bar
            this->setMinimum(gauge.minGaugeValue());
            this->setMaximum(max);
            this->setValue(val);
            this->setFormat("%v/%m ");
            this->setToolTip(
                QStringLiteral(u"%1 (%2/%3)")
                    .arg(gauge.name())
                    .arg(val)
                    .arg(max)
            );
            
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