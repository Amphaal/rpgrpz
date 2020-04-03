// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QVariantHash>
#include <QMap>

class RPZInventorySlot : public QVariantHash {
    public:
        RPZInventorySlot() {}
        explicit RPZInventorySlot(const QVariantHash &hash) : QVariantHash(hash) {}

        const QString id() const {
            return this->name();
        }

        void setName(const QString &name) {this->insert(QStringLiteral(u"n"), name);}
        const QString name() const {return this->value(QStringLiteral(u"n")).toString().trimmed();} 

        void setCategory(const QString &archtype) { this->insert(QStringLiteral(u"c"), archtype); }
        const QString category() const {return this->value(QStringLiteral(u"c")).toString();}

        void setDescription(const QString &descr) { this->insert(QStringLiteral(u"d"), descr); }
        const QString description() const {return this->value(QStringLiteral(u"d")).toString();}

        void setHowMany(const uint &number) {this->insert(QStringLiteral(u"nb"), number);}
        uint howMany() const {return this->value(QStringLiteral(u"nb")).toUInt();}

        void setWeight(const double &weight) {this->insert(QStringLiteral(u"w"), weight);}
        double weight() const {return this->value(QStringLiteral(u"w")).toDouble();}
};

class RPZInventory : public QVariantHash {
    public:
        RPZInventory() {}
        explicit RPZInventory(const QVariantHash &hash) : QVariantHash(hash) {}

        const QString toString() const {
             auto nameStr = this->name();
             return nameStr.isEmpty() ? QObject::tr("New Inventory") : nameStr;
        }

        void setName(const QString &name) {this->insert(QStringLiteral(u"n"), name);}
        const QString name() const {return this->value(QStringLiteral(u"n")).toString();} 

        void setInventorySlots(const QMap<QString, RPZInventorySlot> &iSlots) {
            QVariantMap in;
            for(auto i = iSlots.begin(); i != iSlots.end(); i++) {
                in.insert(i.key(), i.value());
            }
            this->insert(QStringLiteral(u"s"), in);
        };
        
        const QMap<QString, RPZInventorySlot> inventorySlots() const {
            QMap<QString, RPZInventorySlot> out;

            auto mapped = this->value(QStringLiteral(u"s")).toMap();
            for(auto i = mapped.begin(); i != mapped.end(); i++) {
                auto slot = RPZInventorySlot(i.value().toHash());
                out.insert(i.key(), slot);
            }

            return out;
        };
};
