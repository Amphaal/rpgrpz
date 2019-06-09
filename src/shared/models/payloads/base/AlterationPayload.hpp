#pragma once

#include <QVariantHash>
#include "libs/snowflake.hpp"

enum PayloadAlteration {
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
    Undone
}; 

class AlterationPayload : public QVariantHash { 
    public:
        enum Source {
            Undefined,
            Local_MapLayout,
            Local_Map,
            Network
        };

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

    private:      
        bool _isNetworkAlteration = false;
        bool _instructGIBuild = false;

        static inline const QList<PayloadAlteration> _networkAlterations = { 
            Moved, 
            Added, 
            Removed, 
            Reset,
            LayerChanged,
            TextChanged,
            Rotated,
            Scaled,
            LockChanged,
            VisibilityChanged 
        };
        
        static inline const QList<PayloadAlteration> _networkAlterations = { 
            Moved, 
            Added, 
            Removed, 
            Reset,
            LayerChanged,
            TextChanged,
            Rotated,
            Scaled,
            LockChanged,
            VisibilityChanged 
        };

        static const inline const QList<PayloadAlteration> _buildGraphicsItemAlterations = {
            Added,  
            Reset
        };

        void _setType(const PayloadAlteration &type) {
            (*this)["t"] = (int)type;
            this->_updateTags(type);
        }

        void _updateTags(const PayloadAlteration &type) {
            this->_isNetworkAlteration = _networkAlterations.contains(type);
            this->_instructGIBuild = _buildGraphicsItemAlterations.contains(type);
        }
};