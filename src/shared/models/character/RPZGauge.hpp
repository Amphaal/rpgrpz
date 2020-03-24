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