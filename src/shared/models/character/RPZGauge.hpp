#pragma once

#include <QVariantHash>
#include <QColor>

class RPZGauge : public QVariantHash {
    public:
        RPZGauge() {}
        RPZGauge(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert("n", name);}
        const QString name() const {return this->value("n").toString();} 

        void setColor(const QColor &color) {this->insert("c", color.name());}
        const QColor color() const { return QColor(this->value("c", "#00008B").toString());}

        void setVisibleUnderPortrait(const bool &isVisible) {this->insert("v", isVisible);}
        const bool isVisibleUnderPortrait() const { return this->value("v").toBool();}

        void setMinGaugeValue(const int &number) {this->insert("min", number);}
        const int minGaugeValue() const {return this->value("min", 0).toInt();}

        void setMaxGaugeValue(const int &number) {this->insert("max", number);}
        const int maxGaugeValue() const {return this->value("max", 30).toInt();}

        void setGaugeValue(const int &number) {this->insert("val", number);}
        const int gaugeValue() const {return this->value("val", 15).toInt();}
};