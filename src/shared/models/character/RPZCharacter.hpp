#pragma once

#include <QVariantHash>
#include <QPixmap>
#include <QBuffer>

#include "../base/Serializable.hpp"

#include "RPZAbility.hpp"
#include "RPZGauge.hpp"
#include "RPZInventory.hpp"

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
        
        static inline const QSize defaultPortraitSize = QSize(240, 320);

        static const QPixmap getPortrait(const RPZCharacter &character) {
            auto portrait = character.portrait();
            return portrait.isNull() ? getDefaultPortrait() : portrait;
        }

        //

        void setPortrait(const QPixmap &portrait, const QString &ext) {
            
            //prepare to write
            QByteArray bArray;
            QBuffer buffer(&bArray);

            //write into buffer
            buffer.open(QIODevice::WriteOnly);
                portrait.save(&buffer, ext.toStdString().c_str());
            buffer.close();
            
            auto base64Img = bArray.toBase64();
            auto utf8Img = QString::fromUtf8(base64Img);

            this->insert("img", utf8Img);
            this->insert("img_ext", ext);

        }
        const QPixmap portrait() const { 
            
            auto imgDataStr = this->value("img").toString();
            auto imgDataBase64 = imgDataStr.toUtf8();
            auto imgData = QByteArray::fromBase64(imgDataBase64); 

            auto ext = this->value("img_ext").toString();
            
            QPixmap out;
            out.loadFromData(imgData, ext.toStdString().c_str());
            return out;

        }
        
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

        void setGauges(const QVector<RPZGauge> &gauges) {
            QVariantList in;
            for(auto &gauge : gauges) in += gauge;
            this->insert("g", in);
        }
        const QVector<RPZGauge> gauges() const {
            QVector<RPZGauge> out;
            for(auto &gauge : this->value("g").toList()) out += RPZInventory(gauge.toHash());
            return out;
        }

        void setAbilities(const QVector<RPZAbility> &abilities) {
            QVariantList in;
            for(auto &ability : abilities) in += ability;
            this->insert("ab", in);
        }
        const QVector<RPZAbility> abilities() const {
            QVector<RPZAbility> out;
            for(auto &ability : this->value("ab").toList()) out += RPZAbility(ability.toHash());
            return out;
        }

        //

        void setInventories(const QVector<RPZInventory> &inventories) {
            QVariantList in;
            for(auto &inventory : inventories) in += inventory;
            this->insert("inv", in);
        }
        const QVector<RPZInventory> inventories() const {
            QVector<RPZInventory> out;
            for(auto &inv : this->value("inv").toList()) out += RPZInventory(inv.toHash());
            return out;
        }

        //
        void setNotes(const QString &notes) {this->insert("notes", notes);}
        const QString notes() const {return this->value("notes").toString();}

        static const QPixmap getDefaultPortrait() {
            if(!_defaultPortrait) _defaultPortrait = new QPixmap(":/asset/default.jpg");
            return *_defaultPortrait;
        }

    private:
        static inline QPixmap* _defaultPortrait = nullptr;

};