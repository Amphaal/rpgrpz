#pragma once

#include <QVariantHash>
#include "libs/snowflake.hpp"

enum class PayloadAlteration {
    Unknown,
    Focused,
    Selected,
    Removed,
    LayerChanged,
    Moved, 
    Added, 
    Reset,
    TextChanged,
    Rotated,
    Scaled,
    LockChanged,
    Duplicated,
    VisibilityChanged,
    Redone,
    Undone,
    OwnerChanged
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

        virtual QVariantMap alterationByAtomId() {
            return QVariantMap();
        }

        void changeSource(const Source &newSource) {
            (*this)["s"] = (int)newSource;
        }

        Source source() {
            return (Source)this->value("s").toInt();
        }

        bool requiresGraphicsItemBuild() {
            return this->_instructGIBuild;
        }

        bool isNetworkRoutable() {
            return this->_isNetworkAlteration;
        }

        bool isRedoCompatible() {
            return this->_isRedoCompatible;
        }

    private:      
        bool _isNetworkAlteration = false;
        bool _instructGIBuild = false;
        bool _isRedoCompatible = false;

        static inline const QList<PayloadAlteration> _networkAlterations = { 
            PayloadAlteration::Moved, 
            PayloadAlteration::Added, 
            PayloadAlteration::Removed, 
            PayloadAlteration::Reset,
            PayloadAlteration::LayerChanged,
            PayloadAlteration::TextChanged,
            PayloadAlteration::Rotated,
            PayloadAlteration::Scaled,
            PayloadAlteration::LockChanged,
            PayloadAlteration::VisibilityChanged 
        };
        
        static inline const QList<PayloadAlteration> _redoAlterations = { 
            PayloadAlteration::Moved, 
            PayloadAlteration::Added, 
            PayloadAlteration::Removed
        };

        static inline const QList<PayloadAlteration> _buildGraphicsItemAlterations = {
            PayloadAlteration::Added,  
            PayloadAlteration::Reset
        };

        void _setType(const PayloadAlteration &type) {
            (*this)["t"] = (int)type;
            this->_updateTags(type);
        }

        void _updateTags(const PayloadAlteration &type) {
            this->_isNetworkAlteration = _networkAlterations.contains(type);
            this->_instructGIBuild = _buildGraphicsItemAlterations.contains(type);
            this->_isRedoCompatible = _redoAlterations.contains(type);
        }
};