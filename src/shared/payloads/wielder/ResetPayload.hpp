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
// different license and copyright still refer to this GPL.

#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class ResetPayload : public AtomsWielderPayload {
    public:
        ResetPayload() {}
        explicit ResetPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        ResetPayload(const MapDatabase &map) : AtomsWielderPayload(map) {
            this->_setMapParams(map.mapParams());
            this->setFogParams(map.fogParams());
        }
        
        friend QDebug operator<<(QDebug debug, const ResetPayload &c) {
            QDebugStateSaver saver(debug);
            debug.nospace() << c.type() << ", count:" << c.atoms().count();
            return debug;
        }
    
        const RPZMapParameters mapParameters() const {
            return RPZMapParameters(this->value("mParams").toHash());
        }
        
        const RPZFogParams fogParameters() const {
            return RPZFogParams(this->value("fParams").toHash());
        };

        void setMapParams(const RPZMapParameters &mapParams) {
            this->insert("fromMPUpdate", true);
            this->_setMapParams(mapParams);
        }

        void setFogParams(const RPZFogParams &fogParams) {
            this->insert("fParams", fogParams);
        }

        bool isFromMapParametersUpdate() const {
            return this->value("fromMPUpdate").toBool();
        }

    private:
        void _setMapParams(const RPZMapParameters &mapParams) {
            this->insert("mParams", mapParams);
        }
};