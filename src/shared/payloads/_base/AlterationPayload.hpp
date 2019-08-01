#pragma once

#include <QVariantHash>
#include "src/_libs/snowflake/snowflake.h"

enum PayloadAlteration {
    PA_Unknown,
    PA_Focused,
    PA_Selected,
    PA_Removed,
    PA_Added, 
    PA_Reset,
    PA_Duplicated,
    PA_Redone,
    PA_Undone,
    PA_OwnerChanged,
    PA_MetadataChanged,
    PA_BulkMetadataChanged
}; 

static const QStringList PayloadAlterationAsString {
    "Unknown",
    "Focused",
    "Selected",
    "Removed",
    "Added", 
    "Reset",
    "OwnerChanged",
    "MetadataChanged",
    "BulkMetadataChanged"
};

class AlterationPayload : public QVariantHash { 
    public:
        enum Source {
            Undefined,
            Local_MapLayout,
            Local_Map,
            Local_AtomEditor,
            RPZServer
        };

        static inline QStringList SourceAsStr {
            "Undefined",
            "Local_MapLayout",
            "Local_Map",
            "Local_AtomEditor",
            "RPZServer"
        };

        AlterationPayload() {}
        AlterationPayload(const QVariantHash &hash) : QVariantHash(hash) {
            this->_updateTags(this->type());
        }
        AlterationPayload(const PayloadAlteration &type) : QVariantHash() {
            this->_setType(type);
        }

        PayloadAlteration type() const {
            return (PayloadAlteration)this->value("t").toInt();
        };

        bool hasControllerAcknowledged() const {
            return this->value("ack").toBool();
        }
        void setControllerAck() {
            this->insert("ack", true);
        }

        void changeSource(const Source &newSource) {
            this->insert("s", (int)newSource);
        }

        Source source() const {
            return (Source)this->value("s").toInt();
        }

        bool isNetworkRoutable() const {
            return this->_isNetworkAlteration;
        }

        //necessary for dynamic_cast operations
        virtual ~AlterationPayload() {}

    private:      
        bool _isNetworkAlteration = false;

        static inline const QList<PayloadAlteration> _networkAlterations = { 
            PayloadAlteration::PA_Added, 
            PayloadAlteration::PA_Removed, 
            PayloadAlteration::PA_Reset,
            PayloadAlteration::PA_MetadataChanged,
            PayloadAlteration::PA_BulkMetadataChanged
        };
        
        void _setType(const PayloadAlteration &type) {
            this->insert("t", (int)type);
            this->_updateTags(type);
        }

        void _updateTags(const PayloadAlteration &type) {
            this->_isNetworkAlteration = _networkAlterations.contains(type);
        }
};