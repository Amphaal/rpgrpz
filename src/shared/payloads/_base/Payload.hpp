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
            FogModeChanged
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
    
};