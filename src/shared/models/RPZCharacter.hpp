#pragma once

#include <QVariantHash>
#include "base/Serializable.hpp"

class RPZCharacter : public Serializable {
    public:
        RPZCharacter() {};
        RPZCharacter(const QVariantHash &hash) : Serializable(hash) {};

        const QString toString() const {
            auto out = this->name();
            if(out.isEmpty()) return "Nouveau Personnage [#" + this->idAsStr() + "]";

            auto arch = this->archtype();
            if(!arch.isEmpty()) out += " [" + arch + "]";
            return out;
        }

        //

        void setPortrait(const QByteArray &image) {this->insert("prtrt", image);}
        const QByteArray portrait() const { return this->value("prtrt").toByteArray(); }

        void setName(const QString &name) {this->insert("nm", name);}
        const QString name() const {
            return this->value("nm").toString();
        } 

        void setArchtype(const QString &archtype) { this->insert("arch", archtype); }
        const QString archtype() const {return this->value("arch").toString();}

        void setDescription(const QString &descr) { this->insert("descr", descr); }
        const QString description() const {return this->value("descr").toString();}

        void setStory(const QString &story) {this->insert("stry", story);}
        const QString story() const {return this->value("stry").toString();}

        //

        void setLevel(int level) {this->insert("lvl", level);};
        const int level() const {
            return this->value("lvl", -1).toInt();
        }

        void setBonus(const QString &bonus) {this->insert("bonus", bonus);}
        const QString bonus() const {return this->value("bonus").toString();};

        void setMalus(const QString &malus) {this->insert("malus", malus);}
        const QString malus() const {return this->value("malus").toString();}

        //


        //
        void setNotes(const QString &notes) {this->insert("notes", notes);}
        const QString notes() const {return this->value("notes").toString();}

};