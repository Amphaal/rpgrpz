#pragma once

#include <QVariantHash>
#include <QMap>

class RPZInventorySlot : public QVariantHash {
    public:
        RPZInventorySlot() {}
        RPZInventorySlot(const QVariantHash &hash) : QVariantHash(hash) {}

        const QString id() const {
            return this->name();
        }

        void setName(const QString &name) {this->insert("n", name);}
        const QString name() const {return this->value("n").toString().trimmed();} 

        void setCategory(const QString &archtype) { this->insert("c", archtype); }
        const QString category() const {return this->value("c").toString();}

        void setDescription(const QString &descr) { this->insert("d", descr); }
        const QString description() const {return this->value("d").toString();}

        void setHowMany(const uint &number) {this->insert("nb", number);}
        const uint howMany() const {return this->value("nb").toUInt();}

        void setWeight(const double &weight) {this->insert("w", weight);}
        const double weight() const {return this->value("w").toDouble();}
};

class RPZInventory : public QVariantHash {
    public:
        RPZInventory() {}
        RPZInventory(const QVariantHash &hash) : QVariantHash(hash) {}

        const QString toString() const {
             auto nameStr = this->name();
             return nameStr.isEmpty() ? "Nouvel Inventaire" : nameStr;
        }

        void setName(const QString &name) {this->insert("n", name);}
        const QString name() const {return this->value("n").toString();} 

        void setInventorySlots(const QMap<QString, RPZInventorySlot> &iSlots) {
            QVariantMap in;
            for(auto i = iSlots.begin(); i != iSlots.end(); i++) {
                in.insert(i.key(), i.value());
            }
            this->insert("s", in);
        };
        
        const QMap<QString, RPZInventorySlot> inventorySlots() const {
            QMap<QString, RPZInventorySlot> out;

            auto mapped = this->value("s").toMap();
            for(auto i = mapped.begin(); i != mapped.end(); i++) {
                auto slot = RPZInventorySlot(i.value().toHash());
                out.insert(i.key(), slot);
            }

            return out;
        };
};
