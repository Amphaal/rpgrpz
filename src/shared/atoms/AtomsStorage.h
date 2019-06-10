#pragma once

#include <QHash>
#include <QSet>
#include <QVector>
#include <QStack>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "src/shared/models/entities/RPZAtom.h"
#include "src/shared/models/Payloads.h"

#include "AtomsHandler.h"

class AtomsStorage : public AtomsHandler {

    public:
        AtomsStorage(const AlterationPayload::Source &boundSource);
        
        RPZMap<RPZAtom> atoms();
        
        void redo();
        void undo();

    protected:
        // redo/undo
        QStack<AlterationPayload> _redoHistory;
        QStack<AlterationPayload> _undoHistory;

        int _payloadHistoryIndex = 0;
        void _registerPayloadForHistory(AlterationPayload* payload);
        AlterationPayload _generateUndoPayload(AlterationPayload* historyPayload);

        //duplication
        QVector<snowflake_uid> _latestDuplication;
        int _duplicationCount = 0;
        void _duplicateAtoms(QVector<snowflake_uid> &atomIdList);

        //atoms list 
        RPZMap<RPZAtom> _atomsById;

        //credentials handling
        QHash<snowflake_uid, QSet<snowflake_uid>> _atomIdsByOwnerId;

        //alter the inner atoms lists
        virtual void _handlePayload(AlterationPayload* payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) override;

    private:
        void _basic_handlePayload(AlterationPayload* payload);
};