// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QPaintEvent>
#include <QWidget>
#include <QTextOption>
#include <QPen>
#include <QPainter>

#include "src/shared/models/character/RPZGauge.hpp"

class GaugeWidget : public QWidget {
 public:
        GaugeWidget(const RPZGauge::MinimalistGauge &gauge, const QString &name, const QColor &color) : _name(name), _color(color) {
            this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            this->setFixedHeight(15);
            this->updateValues(gauge);
        }
        explicit GaugeWidget(const RPZGauge &gauge = RPZGauge()) : GaugeWidget(gauge.toMinimalist(), gauge.name(), gauge.color()) {}

        void updateValues(const RPZGauge::MinimalistGauge &values) {
            this->_values = values;

            auto formatedName = this->_name.isEmpty() ? QObject::tr("[Unamed gauge]") : this->_name;
            auto tooltipText = QStringLiteral(u"%1 : %2 / %3").arg(formatedName)
                                                            .arg(values.current)
                                                            .arg(values.max);

            this->setToolTip(tooltipText);
            this->update();
        }

        static void drawGauge(QPainter *painter, const QRectF &gaugeRect, const RPZGauge::MinimalistGauge &gauge, const QColor &gaugeColor) {
            painter->save();

                // text option
                QTextOption tOption(Qt::AlignCenter);

                // pen
                QPen pen;
                pen.setWidth(1);
                pen.setColor("#111");

                // draw outer gauge
                painter->setPen(pen);
                painter->setBrush(QBrush("#FFF", Qt::BrushStyle::SolidPattern));
                painter->drawRoundedRect(gaugeRect, 2, 2);
                painter->setBrush(Qt::NoBrush);

                // determine if gauge must be filled
                auto gVal = gauge.current;
                auto gMax = gVal > -1 ? gauge.max : gauge.min;

                if (gVal) {
                    // rect
                    auto innerGaugeRect = gaugeRect;
                    if (gVal > -1) innerGaugeRect = innerGaugeRect.marginsRemoved(QMargins(1, 1, 0, 0));

                    // pattern
                    auto pattern = gVal > -1 ? Qt::BrushStyle::SolidPattern : Qt::BrushStyle::FDiagPattern;
                    auto fillBrush = QBrush(gaugeColor, pattern);

                    // matrix
                    if (gVal < 0) {
                        QTransform matrix;
                        matrix.scale(.5, .5);
                        fillBrush.setTransform(matrix);
                    }

                    // calculate gauge ratio
                    double gaugeRatio = 0;
                    if (gMax) {
                        gaugeRatio = (double)gVal / gMax;
                    }

                    // reduce innerGaugeRect
                    innerGaugeRect.setWidth(
                        (int)(gaugeRatio * innerGaugeRect.width())
                    );

                    // if negative, move to right border
                    if (gVal < 0) innerGaugeRect.moveTopRight(gaugeRect.topRight());

                    painter->setOpacity(.75);

                    // print gauge indicator
                    painter->fillRect(innerGaugeRect, fillBrush);
                }

                // define content
                auto textContent = QStringLiteral(u"%1 / %2").arg(gVal).arg(gMax);

                // draw text
                pen.setColor("#000");
                painter->setPen(pen);
                painter->setOpacity(1);
                painter->drawText(gaugeRect, textContent, tOption);

            painter->restore();
        }

        static void drawGauge(QPainter *painter, const QRectF &gaugeRect, const RPZGauge &gauge) {
            drawGauge(painter, gaugeRect, gauge.toMinimalist(), gauge.color());
        }

 private:
        RPZGauge::MinimalistGauge _values;
        QString _name;
        QColor _color;

        void paintEvent(QPaintEvent *event) override {
            QPainter p(this);
            drawGauge(&p, event->rect().marginsRemoved(QMargins(1, 1, 1, 1)), this->_values, this->_color);
        }
};
