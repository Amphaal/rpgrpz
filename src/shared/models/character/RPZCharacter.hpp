#pragma once

#include <QVariantHash>
#include <QPixmap>
#include <QBuffer>

#include "src/shared/models/_base/Serializable.hpp"

#include "RPZAbility.hpp"
#include "RPZGauge.hpp"
#include "RPZInventory.hpp"

class RPZCharacter : public Serializable {
    public:
        typedef SnowFlake::Id Id;

        RPZCharacter() {};
        explicit RPZCharacter(const QVariantHash &hash) : Serializable(hash) {};

        const QString toString() const {
            auto out = this->name();
            if(out.isEmpty()) return QObject::tr("New character [#%1]").arg(this->idAsStr());

            auto arch = this->archtype();
            if(!arch.isEmpty()) out += QStringLiteral(u" [%1]").arg(arch);
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
                portrait.save(&buffer, qUtf8Printable(ext));
            buffer.close();
            
            auto base64Img = bArray.toBase64();
            auto utf8Img = QString::fromUtf8(base64Img);

            this->insert(QStringLiteral(u"img"), utf8Img);
            this->insert(QStringLiteral(u"img_ext"), ext);

        }
        const QPixmap portrait() const { 
            
            auto imgDataStr = this->value(QStringLiteral(u"img")).toString();
            auto imgDataBase64 = imgDataStr.toUtf8();
            auto imgData = QByteArray::fromBase64(imgDataBase64); 

            auto ext = this->value(QStringLiteral(u"img_ext")).toString();
            
            QPixmap out;
            out.loadFromData(imgData, qUtf8Printable(ext));
            return out;

        }
        
        void setName(const QString &name) {this->insert(QStringLiteral(u"nm"), name);}
        const QString name() const {
            return this->value(QStringLiteral(u"nm")).toString();
        } 

        void setArchtype(const QString &archtype) { this->insert(QStringLiteral(u"arch"), archtype); }
        const QString archtype() const {return this->value(QStringLiteral(u"arch")).toString();}

        void setDescription(const QString &descr) { this->insert(QStringLiteral(u"descr"), descr); }
        const QString description() const {return this->value(QStringLiteral(u"descr")).toString();}

        void setStory(const QString &story) {this->insert(QStringLiteral(u"stry"), story);}
        const QString story() const {return this->value(QStringLiteral(u"stry")).toString();}

        //

        void setLevel(int level) {this->insert(QStringLiteral(u"lvl"), level);};
        const int level() const {
            return this->value(QStringLiteral(u"lvl"), -1).toInt();
        }

        void setBonus(const QString &bonus) {this->insert(QStringLiteral(u"bonus"), bonus);}
        const QString bonus() const {return this->value(QStringLiteral(u"bonus")).toString();};

        void setMalus(const QString &malus) {this->insert(QStringLiteral(u"malus"), malus);}
        const QString malus() const {return this->value(QStringLiteral(u"malus")).toString();}

        void setGauges(const QVector<RPZGauge> &gauges) {
            
            QVariantList in;
            int favGaugesCount = 0;
            
            for(const auto &gauge : gauges) {
                if(gauge.isVisibleUnderPortrait()) favGaugesCount++;
                in += gauge;
            }

            this->insert(QStringLiteral(u"g"), in);
            this->insert(QStringLiteral(u"gfavc"), favGaugesCount);

        }
        const QVector<RPZGauge> gauges() const {
            QVector<RPZGauge> out;
            for(const auto &gauge : this->value(QStringLiteral(u"g")).toList()) out += RPZGauge(gauge.toHash());
            return out;
        }
        const int favGaugesCount() const {
            return this->value(QStringLiteral(u"gfavc"), 0).toInt();
        }

        void setAbilities(const QVector<RPZAbility> &abilities) {
            QVariantList in;
            for(const auto &ability : abilities) in += ability;
            this->insert(QStringLiteral(u"ab"), in);
        }
        const QVector<RPZAbility> abilities() const {
            QVector<RPZAbility> out;
            for(const auto &ability : this->value(QStringLiteral(u"ab")).toList()) out += RPZAbility(ability.toHash());
            return out;
        }

        //

        void setInventories(const QVector<RPZInventory> &inventories) {
            QVariantList in;
            for(const auto &inventory : inventories) in += inventory;
            this->insert(QStringLiteral(u"inv"), in);
        }
        const QVector<RPZInventory> inventories() const {
            QVector<RPZInventory> out;
            for(const auto &inv : this->value(QStringLiteral(u"inv")).toList()) out += RPZInventory(inv.toHash());
            return out;
        }

        //
        void setNotes(const QString &notes) {this->insert(QStringLiteral(u"notes"), notes);}
        const QString notes() const {return this->value(QStringLiteral(u"notes")).toString();}

        static const QPixmap getDefaultPortrait() {
            if(!_defaultPortrait) _defaultPortrait = new QPixmap(":/assets/default.jpg");
            return *_defaultPortrait;
        }

    private:
        static inline QPixmap* _defaultPortrait = nullptr;

};