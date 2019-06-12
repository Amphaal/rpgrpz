#pragma once

#include <QVariantHash>
#include "libs/snowflake.hpp"

enum class PayloadAlteration {
    Unknown,
    Focused,
    Selected,
    Removed,
    Added, 
    Reset,
    Duplicated,
    Redone,
    Undone,
    OwnerChanged,
    MetadataChanged,
    BulkMetadataChanged
}; 

class AlterationPayload : public QVariantHash { 
    public:
        enum Source {
            Undefined,
            Local_MapLayout,
            Local_Map,
            Network
        };

        AlterationPayload() {}
        AlterationPayload(const QVariantHash &hash) : QVariantHash(hash) {
            this->_updateTags(this->type());
        }
        AlterationPayload(const PayloadAlteration &type) : QVariantHash() {
            this->_setType(type);
        }

        PayloadAlteration type() {
            return (PayloadAlteration)this->value("t").toInt();
        };

        void changeSource(const Source &newSource) {
            (*this)["s"] = (int)newSource;
        }

        Source source() {
            return (Source)this->value("s").toInt();
        }

        bool isNetworkRoutable() {
            return this->_isNetworkAlteration;
        }

        //necessary for dynamic_cast operations
        virtual ~AlterationPayload() {
            QVariantHash::~QVariantHash();
        }

    private:      
        bool _isNetworkAlteration = false;

        static inline const QList<PayloadAlteration> _networkAlterations = { 
            PayloadAlteration::Added, 
            PayloadAlteration::Removed, 
            PayloadAlteration::Reset,
            PayloadAlteration::MetadataChanged,
            PayloadAlteration::BulkMetadataChanged
        };
        
        void _setType(const PayloadAlteration &type) {
            (*this)["t"] = (int)type;
            this->_updateTags(type);
        }

        void _updateTags(const PayloadAlteration &type) {
            this->_isNetworkAlteration = _networkAlterations.contains(type);
        }
};