#pragma once

#include <QVariantHash>

class RPZAbility : public QVariantHash {
    public:
        RPZAbility() {}
        RPZAbility(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert("n", name);}
        const QString name() const {return this->value("n").toString();} 

        void setCategory(const QString &archtype) { this->insert("c", archtype); }
        const QString category() const {return this->value("c").toString();}

        void setDescription(const QString &descr) { this->insert("d", descr); }
        const QString description() const {return this->value("d").toString();}

        void setFavorite(const bool &isFavorite) {this->insert("f", isFavorite);}
        const bool isFavorite() const { return this->value("f").toBool();}
};