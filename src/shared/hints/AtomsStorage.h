#pragma once

#include <QHash>
#include <set>
#include <QVector>
#include <QStack>
#include <QVariantList>
#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

#include "src/shared/async-ui/AlterationAcknoledger.h"

#include "src/shared/database/MapDatabase.h"

struct AtomsSelectionDescriptor {
    QSet<RPZAtomType> representedTypes;
    RPZAtom templateAtom;
    QVector<RPZAtomId> selectedAtomIds;
};

struct PossibleActionsOnAtomList {
    bool somethingRedoable = false;
    bool somethingUndoable = false;
    bool canCopy = false;
    bool canRemove = false;
    bool canChangeLayer = false;
    bool canChangeVisibility = false;
    bool canChangeAvailability = false;
    int targetUpLayer = 0;
    int targetDownLayer = 0;
};

class AtomsStorage : public AlterationAcknoledger {

    Q_OBJECT

    public:
        AtomsStorage(const Payload::Source &boundSource);
        
        QVector<RPZAtomId> bufferedSelectedAtomIds() const; //safe
        const AtomsSelectionDescriptor getAtomSelectionDescriptor(const QVector<RPZAtomId> &selectedIds) const; //safe
        
        PossibleActionsOnAtomList getPossibleActions(const QVector<RPZAtomId> &ids);

        const ResetPayload generateResetPayload() const;

    public slots:    
        void redo();
        void undo();
        void duplicateAtoms(const QVector<RPZAtomId> &RPZAtomIdList);
        void handleAlterationRequest(const AlterationPayload &payload);

    protected:
        MapDatabase _map;

        virtual void _handleAlterationRequest(const AlterationPayload &payload) override;
        virtual void _atomAdded(const RPZAtom &added) {};

        virtual void _basicAlterationDone(const QList<RPZAtomId> &updatedIds, const Payload::Alteration &type) {};
        virtual void _updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) {};
        virtual void _updatesDone(const AtomsUpdates &updates) {};

    private:
        mutable QMutex _m_handlingLock;

        // redo/undo
        QStack<AlterationPayload> _redoHistory;
        QStack<AlterationPayload> _undoHistory;
        int _payloadHistoryIndex = 0;
        void _registerPayloadForHistory(const AlterationPayload &payload);
        AlterationPayload _generateUndoPayload(const AlterationPayload &historyPayload);
        int _canRedo();
        int _canUndo();

        //selected
        QSet<RPZAtomId> _selectedRPZAtomIds;

        //duplication
        int _duplicationCount = 0;
        QVector<RPZAtomId> _latestDuplication;
        RPZMap<RPZAtom> _generateAtomDuplicates(const QVector<RPZAtomId> &RPZAtomIdsToDuplicate) const;
        static constexpr int _pixelStepPosDuplication = 10;
        static QPointF _getPositionFromAtomDuplication(const RPZAtom &atomToDuplicate, int duplicateCount);

        //
        QPair<int, int> _determineMinMaxLayer(const QList<const RPZAtom*> &atoms);
};