#pragma once

#include <QVariantHash>

class RPZAbility : public QVariantHash {
    public:
        RPZAbility() {}
        explicit RPZAbility(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert(QStringLiteral(u"n"), name);}
        const QString name() const {return this->value(QStringLiteral(u"n")).toString();} 

        void setCategory(const QString &archtype) { this->insert(QStringLiteral(u"c"), archtype); }
        const QString category() const {return this->value(QStringLiteral(u"c")).toString();}

        void setDescription(const QString &descr) { this->insert(QStringLiteral(u"d"), descr); }
        const QString description() const {return this->value(QStringLiteral(u"d")).toString();}

        void setFavorite(const bool &isFavorite) {this->insert(QStringLiteral(u"f"), isFavorite);}
        const bool isFavorite() const { return this->value(QStringLiteral(u"f")).toBool();}
};