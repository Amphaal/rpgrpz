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

// Any graphical or audio resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "AtomRelatedPayload.hpp"

#include <QList>

class MultipleAtomTargetsPayload : public AtomRelatedPayload {
    public:   
        QList<RPZAtom::Id> targetRPZAtomIds() const {
            
            auto list = this->value(QStringLiteral(u"ids")).toList();

            QList<RPZAtom::Id> out;
            for(const auto &e : list) {
                auto id = e.toULongLong();
                out.append(id);
            }
            
            return out;
        }

        virtual QVariant args() const {
            return this->value(QStringLiteral(u"args"));
        }

        AtomRelatedPayload::RemainingAtomIds restrictTargetedAtoms(const QSet<RPZAtom::Id> &idsToRemove) override {
            
            auto targeted = this->targetRPZAtomIds();

            for(const auto &id : targeted) {
                targeted.removeOne(id);
            }

            this->_setTargetRPZAtomIds(targeted);

            return targeted.count();

        };

    protected:
        explicit MultipleAtomTargetsPayload(const QVariantHash &hash) : AtomRelatedPayload(hash) {}
        MultipleAtomTargetsPayload(const Payload::Alteration &alteration, const QList<RPZAtom::Id> &targetedRPZAtomIds) : AtomRelatedPayload(alteration) {
            this->_setTargetRPZAtomIds(targetedRPZAtomIds);
        }
    
    private:
        void _setTargetRPZAtomIds(const QList<RPZAtom::Id> &targetRPZAtomIds) {
            QVariantList cast;
            for(const auto &id : targetRPZAtomIds) {
                cast.append(
                    QString::number(id)
                );
            }
            this->insert(QStringLiteral(u"ids"), cast);
        }
};