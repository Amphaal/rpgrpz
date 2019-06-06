#pragma once

#include <QVariantHash>
#include "libs/snowflake.hpp"

class AlterationPayload : public QVariantHash { 
    public:
        enum Source {
            Undefined,
            Local_MapLayout,
            Local_Map,
            Network
        };

        enum Alteration {
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
            VisibilityChanged
        }; 

        AlterationPayload(const QVariantHash &hash) : QVariantHash(hash) {
            this->_updateTags(this->type());
        }
        AlterationPayload(const Alteration &type) : QVariantHash() {
            this->_setType(type);
        }

        Alteration type() {
            return (Alteration)this->value("t").toInt();
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

        static inline const QList<Alteration> _networkAlterations = { 
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

        static const inline QList<Alteration> _buildGraphicsItemAlterations = {
            Added,  
            Reset
        };

        void _setType(const Alteration &type) {
            (*this)["t"] = (int)type;
            this->_updateTags(type);
        }

        void _updateTags(const Alteration &type) {
            this->_isNetworkAlteration = _networkAlterations.contains(type);
            this->_instructGIBuild = _buildGraphicsItemAlterations.contains(type);
        }
};