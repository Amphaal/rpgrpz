#pragma once

#include <QProgressBar>

#include "src/shared/models/character/RPZGauge.hpp"

class GaugeWidget : public QProgressBar {
    public:
        GaugeWidget(const RPZGauge::MinimalistGauge &gauge, const QString &name, const QColor &color) : _name(name) {

            this->updateValues(gauge);
            
            //apply colors
            auto formatted = _styleTemplate.arg(color.name());
            this->setStyleSheet(formatted);

        }

        GaugeWidget(const RPZGauge &gauge = RPZGauge()) : GaugeWidget(gauge.toMinimalist(), gauge.name(), gauge.color()) {}

        void updateValues(const RPZGauge::MinimalistGauge &gauge) {
            
            this->setMinimum(gauge.min);
            this->setMaximum(gauge.max);
            this->setValue(gauge.current);

            this->setToolTip(
                QStringLiteral(u"%1 (%2/%3)")
                    .arg(this->_name)
                    .arg(gauge.current)
                    .arg(gauge.max)
            );
        }
    
    private:
        QString _name;

        static inline QString _styleTemplate = "QProgressBar {border: 1px solid grey; border-radius: 3px; text-align: right;} QProgressBar::chunk {background-color: %1;width: 20px;}";

};