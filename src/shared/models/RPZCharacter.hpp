#pragma once

#include <QVariantHash>
#include "base/Serializable.hpp"

class RPZCharacter : public Serializable {
    public:
        RPZCharacter() {};
        RPZCharacter(const QVariantHash &hash) : Serializable(hash) {};

        const QString name() const {
            return this->value("nm", "Nouveau Personnage [#" + this->idAsStr() + "]").toString();
        } 

        const QString archtype() const {
            return this->value("arch").toString();
        }

        const QString toString() const {
            auto out = this->name();
            auto arch = this->archtype();
            if(!arch.isEmpty()) out += "[" + arch + "]";
            return out;
        }

        const int level() const {
            return this->value("lvl", -1).toInt();
        }

};