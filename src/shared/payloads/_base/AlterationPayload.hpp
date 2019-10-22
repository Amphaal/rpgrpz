#pragma once

#include <QVariantHash>

#include "src/_libs/snowflake/snowflake.h"
#include "src/shared/models/RPZAtom.h"

enum class PayloadAlteration {
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
    AssetSelected
}; 

static const QStringList PayloadAlterationAsStr {
    "Unknown",
    "Focused",
    "Selected",
    "Removed",
    "Added", 
    "Reset",
    "MetadataChanged",
    "BulkMetadataChanged",
    "AssetChanged",
    "AtomTemplateChanged",
    "AtomTemplateSelected",
    "AssetSelected"
};

class AlterationPayload : public QVariantHash { 
    
    public:
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

        static inline QStringList SourceAsStr {
            "Undefined",
            "Local_MapLayout",
            "Local_Map",
            "Local_AtomEditor",
            "Local_AtomDB",
            "RPZServer",
            "RPZClient",
            "Local_System"
        };

        AlterationPayload() {}
        AlterationPayload(const QVariantHash &hash) : QVariantHash(hash) {}
        AlterationPayload(const PayloadAlteration &type) : QVariantHash() {
            this->_setType(type);
        }

        PayloadAlteration type() const {
            return (PayloadAlteration)this->value(QStringLiteral(u"t")).toInt();
        };

        void changeSource(const Source &newSource) {
            this->insert(QStringLiteral(u"s"), (int)newSource);
        }

        Source source() const {
            return (Source)this->value(QStringLiteral(u"s")).toInt();
        }

        bool isNetworkRoutable() const {
            return _networkAlterations.contains(this->type());
        }

        void tagAsFromTimeline() {
            this->insert(QStringLiteral(u"h"), true);
        }

        bool isFromTimeline() const {
            return this->value(QStringLiteral(u"h")).toBool();
        }

        //necessary for dynamic_cast operations
        virtual ~AlterationPayload() {}

    private:      
        bool _isFromTimeline = false; //client only

        static inline const QList<PayloadAlteration> _networkAlterations = { 
            PayloadAlteration::Added, 
            PayloadAlteration::Removed, 
            PayloadAlteration::Reset,
            PayloadAlteration::MetadataChanged,
            PayloadAlteration::BulkMetadataChanged
        };
        
        void _setType(const PayloadAlteration &type) {
            this->insert(QStringLiteral(u"t"), (int)type);
        }
};

Q_DECLARE_METATYPE(PayloadAlteration)
Q_DECLARE_METATYPE(AlterationPayload)