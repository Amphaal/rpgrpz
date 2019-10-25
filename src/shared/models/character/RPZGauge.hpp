#pragma once

#include <QVariantHash>
#include <QColor>

class RPZGauge : public QVariantHash {
    public:
        RPZGauge() {}
        explicit RPZGauge(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert(QStringLiteral(u"n"), name);}
        const QString name() const {return this->value(QStringLiteral(u"n")).toString();} 

        void setColor(const QColor &color) {this->insert(QStringLiteral(u"c"), color.name());}
        const QColor color() const { return QColor(this->value(QStringLiteral(u"c"), "#00008B").toString());}

        void setVisibleUnderPortrait(const bool &isVisible) {this->insert(QStringLiteral(u"v"), isVisible);}
        const bool isVisibleUnderPortrait() const { return this->value(QStringLiteral(u"v")).toBool();}

        void setMinGaugeValue(const int &number) {this->insert(QStringLiteral(u"min"), number);}
        const int minGaugeValue() const {return this->value(QStringLiteral(u"min"), 0).toInt();}

        void setMaxGaugeValue(const int &number) {this->insert(QStringLiteral(u"max"), number);}
        const int maxGaugeValue() const {return this->value(QStringLiteral(u"max"), 30).toInt();}

        void setGaugeValue(const int &number) {this->insert(QStringLiteral(u"val"), number);}
        const int gaugeValue() const {return this->value(QStringLiteral(u"val"), 15).toInt();}
};