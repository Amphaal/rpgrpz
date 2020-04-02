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

#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QString>

class AtomTemplateChangedPayload : public AlterationPayload {
    public:
        explicit AtomTemplateChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        AtomTemplateChangedPayload(const RPZAtom::Updates &changes) : AlterationPayload(Payload::Alteration::AtomTemplateChanged) {
            this->_setMetadataChanges(changes);
        }

        RPZAtom::Updates updates() const {
            
            RPZAtom::Updates out;
            auto base = this->value(QStringLiteral(u"changes")).toHash();  

            for (auto i = base.begin(); i != base.end(); ++i) {
                auto param = (RPZAtom::Parameter)i.key().toInt();
                out.insert(param, i.value());
            }

            return out;

        }

    private:
        void _setMetadataChanges(const RPZAtom::Updates &changes) {
            
            QVariantHash in;
            
            for (auto i = changes.constBegin(); i != changes.constEnd(); ++i) {
                in.insert(QString::number((int)i.key()), i.value());
            }

            this->insert(QStringLiteral(u"changes"), in);

        }
};