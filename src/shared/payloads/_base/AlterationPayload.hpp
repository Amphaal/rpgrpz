#pragma once

#include <QVariantHash>

#include "src/_libs/snowflake/snowflake.h"
#include "src/shared/models/RPZAtom.h"

#include "Payload.hpp"

class AlterationPayload : public QVariantHash { 

    public:
        AlterationPayload() {}
        AlterationPayload(const QVariantHash &hash) : QVariantHash(hash) {}
        AlterationPayload(const Payload::Alteration &type) : QVariantHash() {
            this->_setType(type);
        }

        Payload::Alteration type() const {
            return (Payload::Alteration)this->value(QStringLiteral(u"t")).toInt();
        }

        void changeSource(const Payload::Source &newSource) {
            this->insert(QStringLiteral(u"s"), (int)newSource);
        }

        Payload::Source source() const {
            return (Payload::Source)this->value(QStringLiteral(u"s")).toInt();
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

        static inline const QList<Payload::Alteration> _networkAlterations = { 
            Payload::Alteration::Added, 
            Payload::Alteration::Removed, 
            Payload::Alteration::Reset,
            Payload::Alteration::MetadataChanged,
            Payload::Alteration::BulkMetadataChanged
        };
        
        void _setType(const Payload::Alteration &type) {
            this->insert(QStringLiteral(u"t"), (int)type);
        }
};

Q_DECLARE_METATYPE(AlterationPayload)

inline uint qHash(const Payload::Source &key, uint seed = 0) {return uint(key) ^ seed;}
