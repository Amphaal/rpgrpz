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

    signals: 
        void heavyAlterationProcessing();

    public slots:    
        void redo();
        void undo();
        void duplicateAtoms(const QVector<snowflake_uid> &atomIdList);
        void handleAlterationRequest(AlterationPayload &payload);

    protected:
        void _bindDefaultOwner(const RPZUser &newOwner);

        virtual void _handleAlterationRequest(AlterationPayload &payload) override;
        virtual void _atomsCreated();

        virtual RPZAtom* _insertAtom(const RPZAtom &newAtom);
        snowflake_uid _ackSelection(RPZAtom* selectedAtom);
        virtual RPZAtom* _changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner);
        snowflake_uid _removeAtom(RPZAtom* toRemove);
        snowflake_uid _updateAtom(RPZAtom* toUpdate, const AtomUpdates &updates);
        
        virtual void basicAlterationDone(const QHash<snowflake_uid, RPZAtom*> &updatedAtoms, const PayloadAlteration &type);
        virtual void updatesDone(const QList<snowflake_uid> &updatedIds, const AtomUpdates &updates);
        virtual void updatesDone(const AtomsUpdates &updates);
        virtual void ownerChangeDone(const QList<RPZAtom*> &updatedAtoms, const RPZUser &newUser);

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