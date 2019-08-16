#pragma once

#include <QHash>
#include <QSet>
#include <QVector>
#include <QStack>
#include <QVariantList>
#include <QObject>
#include <QDebug>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

#include "src\shared\async-ui\AlterationAcknoledger.h"

class AtomsStorage : public AlterationAcknoledger {

    public:
        AtomsStorage(const AlterationPayload::Source &boundSource, bool autoLinkage = true);
        
        RPZMap<RPZAtom> atoms();
        
        void redo();
        void undo();

        void duplicateAtoms(const QVector<snowflake_uid> &atomIdList);
        QVector<snowflake_uid> selectedAtomIds();
        void handleAlterationRequest(AlterationPayload &payload);

    protected:
        // redo/undo
        QStack<AlterationPayload> _redoHistory;
        QStack<AlterationPayload> _undoHistory;

        int _payloadHistoryIndex = 0;
        void _registerPayloadForHistory(AlterationPayload &payload);
        AlterationPayload _generateUndoPayload(AlterationPayload &historyPayload);

        //duplication
        QVector<snowflake_uid> _latestDuplication;
        RPZUser _defaultOwner;
        int _duplicationCount = 0;

        //atoms list 
        RPZMap<RPZAtom> _atomsById;

        //credentials handling
        QHash<snowflake_uid, QSet<snowflake_uid>> _atomIdsByOwnerId;

        //alter the inner atoms lists
        void _handleAlterationRequest(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration);

    private:
        QVector<snowflake_uid> _selectedAtomIds;

        //duplication
        RPZMap<RPZAtom> _generateAtomDuplicates(const QVector<snowflake_uid> &atomIdsToDuplicate) const;
        static constexpr int _pixelStepPosDuplication = 10;
        static QPointF _getPositionFromAtomDuplication(const RPZAtom &atomToDuplicate, int duplicateCount);
};