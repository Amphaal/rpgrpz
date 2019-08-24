#pragma once

#include <QHash>
#include <QSet>
#include <QVector>
#include <QStack>
#include <QVariantList>
#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

#include "src\shared\async-ui\AlterationAcknoledger.h"

class AtomsStorage : public QObject, public AlterationAcknoledger {

    Q_OBJECT

    public:
        AtomsStorage(const AlterationPayload::Source &boundSource, bool autoLinkage = true);
        
        QVector<snowflake_uid> selectedAtomIds() const; //safe
        QVector<RPZAtom*> selectedAtoms() const; //safe
        RPZMap<RPZAtom> atoms() const; //safe

    public slots:    
        void redo();
        void undo();
        void duplicateAtoms(const QVector<snowflake_uid> &atomIdList);
        void handleAlterationRequest(AlterationPayload &payload);

    protected:
        void _handleAlterationRequest(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration);

        void _bindDefaultOwner(const RPZUser &newOwner);

    private:
        mutable QMutex _m_handlingLock;

        //atoms list 
        RPZMap<RPZAtom> _atomsById;
        RPZAtom* _getAtomFromId(const snowflake_uid &id);

        //credentials handling
        QHash<snowflake_uid, QSet<snowflake_uid>> _atomIdsByOwnerId;
        RPZUser _defaultOwner;

        // redo/undo
        QStack<AlterationPayload> _redoHistory;
        QStack<AlterationPayload> _undoHistory;
        int _payloadHistoryIndex = 0;
        void _registerPayloadForHistory(AlterationPayload &payload);
        AlterationPayload _generateUndoPayload(AlterationPayload &historyPayload);

        //selected
        QVector<snowflake_uid> _selectedAtomIds;
        QVector<RPZAtom*> _selectedAtoms;

        //duplication
        int _duplicationCount = 0;
        QVector<snowflake_uid> _latestDuplication;
        RPZMap<RPZAtom> _generateAtomDuplicates(const QVector<snowflake_uid> &atomIdsToDuplicate) const;
        static constexpr int _pixelStepPosDuplication = 10;
        static QPointF _getPositionFromAtomDuplication(const RPZAtom &atomToDuplicate, int duplicateCount);
};