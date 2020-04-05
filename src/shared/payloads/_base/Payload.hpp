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

#include <QObject>

#include "src/shared/models/RPZUser.h"

class Payload {

    Q_GADGET

    public:
        enum class Alteration {
            Unknown,
            Focused,
            Selected,
            Removed,
            Added, 
            Reset,
            MetadataChanged,
            BulkMetadataChanged,
            AssetChanged,
            AtomTemplateChanged,
            AtomTemplateSelected,
            ToySelected,
            FogChanged,
            FogModeChanged,
            Replaced
        }; 
        Q_ENUM(Alteration)

        enum class Source {
            Undefined,
            Local_MapLayout,
            Local_Map,
            Local_AtomEditor,
            Local_AtomDB,
            RPZServer,
            RPZClient,
            Local_System
        };
        Q_ENUM(Source)
        
        static inline const QList<Payload::Alteration> networkAlterations = { 
            Payload::Alteration::Added, 
            Payload::Alteration::Removed, 
            Payload::Alteration::Reset,
            Payload::Alteration::MetadataChanged,
            Payload::Alteration::BulkMetadataChanged,
            Payload::Alteration::FogChanged,
            Payload::Alteration::FogModeChanged
        };

        static inline const QList<Payload::Alteration> allowsRedoUndo = { 
            Payload::Alteration::Added, 
            Payload::Alteration::Removed, 
            Payload::Alteration::MetadataChanged,
            Payload::Alteration::BulkMetadataChanged
        };

        static inline const QList<Payload::Alteration> triggersFoWCheck = { 
            Payload::Alteration::Added, 
            Payload::Alteration::Replaced, 
            Payload::Alteration::MetadataChanged,
            Payload::Alteration::BulkMetadataChanged
        };

        static inline const QList<Payload::Alteration> foWPreventAnimation = { 
            Payload::Alteration::Added, 
            Payload::Alteration::Replaced, 
            Payload::Alteration::Reset
        };
    
};