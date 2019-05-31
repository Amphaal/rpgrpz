#pragma once

#include <QVariantHash>

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
            Reset
        }; 

        static inline const QList<Alteration> networkAlterations = { 
            Moved, 
            Added, 
            Removed, 
            Reset,
            LayerChanged 
        };
        
        static const inline QList<Alteration> buildGraphicsItemAlterations = {
            Added,  
            Reset
        };
        static const inline QList<Alteration> updateGraphicsItemAlterations = {
            Moved, 
            LayerChanged  
        };

        AlterationPayload(const QVariantHash &hash) : QVariantHash(hash) {}
        AlterationPayload(const Alteration &type) : QVariantHash() {
            this->_setType(type);
        }

        Alteration type() {
            return (Alteration)this->value("t").toInt();
        };

        virtual QVariantHash alterationByAtomId() {
            return QVariantHash();
        }

        void changeSource(const Source &newSource) {
            (*this)["s"] = (int)newSource;
        }

        Source source() {
            return (Source)this->value("s").toInt();
        }

    private:       
       void _setType(const Alteration &type) {
           (*this)["t"] = (int)type;
       }
};