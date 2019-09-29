#pragma once

#include <QVariantHash>


class RPZInventorySlot : public QVariantHash {
    public:
        RPZInventorySlot() {}
        RPZInventorySlot(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert("n", name);}
        const QString name() const {return this->value("n").toString();} 

        void setCategory(const QString &archtype) { this->insert("c", archtype); }
        const QString category() const {return this->value("c").toString();}

        void setDescription(const QString &descr) { this->insert("d", descr); }
        const QString description() const {return this->value("d").toString();}

        void setNumber(const int &number) {this->insert("nb", number);}
        const int number() const {return this->value("nb").toInt();}

        void setWeight(const double &weight) {this->insert("w", weight);}
        const double weight() const {return this->value("w").toDouble();}
};

class RPZInventory : public QVariantHash {
    public:
        RPZInventory() {}
        RPZInventory(const QVariantHash &hash) : QVariantHash(hash) {}

        void setName(const QString &name) {this->insert("n", name);}
        const QString name() const {return this->value("n").toString();} 

        void setInventorySlots(const QVector<RPZInventorySlot> &iSlots) {
            QVariantList in;
            for(auto &slot : iSlots) in += slot;
            this->insert("s", in);
        };
        const QVector<RPZInventorySlot> inventorySlots() const {
            QVector<RPZInventorySlot> out;
            for(auto &slot : this->value("s").toList()) out += RPZInventorySlot(slot.toHash());
            return out;
        };
};
