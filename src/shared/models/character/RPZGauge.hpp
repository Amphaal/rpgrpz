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

#include <QVariantHash>
#include <QColor>

class RPZGauge : public QVariantHash {
 public:
        
        struct MinimalistGauge {
            int current; 
            int min; 
            int max;
        };

        RPZGauge() {}
        explicit RPZGauge(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert(QStringLiteral(u"n"), name);}
        const QString name() const {return this->value(QStringLiteral(u"n")).toString();} 

        void setColor(const QColor &color) {this->insert(QStringLiteral(u"c"), color.name());}
        const QColor color() const { return QColor(this->value(QStringLiteral(u"c"), "#00008B").toString());}

        void setVisibleUnderPortrait(const bool &isVisible) {this->insert(QStringLiteral(u"v"), isVisible);}
        bool isVisibleUnderPortrait() const { return this->value(QStringLiteral(u"v")).toBool();}

        void setMinGaugeValue(const int &number) {this->insert(QStringLiteral(u"min"), number);}
        int minGaugeValue() const {return this->value(QStringLiteral(u"min"), 0).toInt();}

        void setMaxGaugeValue(const int &number) {this->insert(QStringLiteral(u"max"), number);}
        int maxGaugeValue() const {return this->value(QStringLiteral(u"max"), 30).toInt();}

        void setGaugeValue(const int &number) {this->insert(QStringLiteral(u"val"), number);}
        int gaugeValue() const {return this->value(QStringLiteral(u"val"), 15).toInt();}

        const RPZGauge::MinimalistGauge toMinimalist() const {
            return {
                this->gaugeValue(),
                this->minGaugeValue(),
                this->maxGaugeValue()
            };
        }
};