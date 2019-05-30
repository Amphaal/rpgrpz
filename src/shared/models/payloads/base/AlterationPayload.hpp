#pragma once

#include <QVariantHash>

class AlterationPayload : protected QVariantHash {
    public:
        enum Source {
            Local,
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
        AlterationPayload(const Alteration &type, const Source &source) : QVariantHash() {
            this->_setType(type);
            this->_setSource(source);
        }

        Alteration type() {
            return (Alteration)this->value("t").toInt();
        };

    protected:
        void _setSource(const Source &source) {
            (*this)["s"] = (int)source;
        }

    private:       
       void _setType(const Alteration &type) {
           (*this)["t"] = (int)type;
       }
};