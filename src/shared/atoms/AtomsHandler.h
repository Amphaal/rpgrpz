#pragma once

#include <QObject>

#include "src/shared/models/Payloads.h"

class AtomsHandler : public QObject {
    
    Q_OBJECT

    public:
        AtomsHandler(const AlterationPayload::Source &boundSource);
        AlterationPayload::Source source();

        void handleAlterationRequest(QVariantHash &payload);

    signals:
        void alterationRequested(QVariantHash &payload);

    protected:
        AlterationPayload::Source _source = AlterationPayload::Source::Undefined;
        
        void _emitAlteration(AlterationPayload* payload);

        virtual void _handlePayload(AlterationPayload* payload) = 0;
        virtual RPZAtom* _handlePayloadInternal(const AlterationPayload::Alteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) = 0; 
};