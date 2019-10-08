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

struct AtomsSelectionDescriptor {
    std::set<AtomType> representedTypes; //prefer std::set to QSet since it handles enum class
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
        AtomsStorage(const AlterationPayload::Source &boundSource);
        
        QVector<RPZAtomId> bufferedSelectedAtomIds() const; //safe
        const AtomsSelectionDescriptor getAtomSelectionDescriptor(const QVector<RPZAtomId> &selectedIds) const; //safe
        
        RPZMap<RPZAtom> atoms() const; //safe
        const QSet<RPZAssetHash> usedAssetIds() const; //safe
        
        ResetPayload createStatePayload() const; //safe
        PossibleActionsOnAtomList getPossibleActions(const QVector<RPZAtomId> &ids);

    public slots:    
        void redo();
        void undo();
        void duplicateAtoms(const QVector<RPZAtomId> &RPZAtomIdList);
        void handleAlterationRequest(AlterationPayload &payload);

    protected:
        RPZAtom* _getAtomFromId(const RPZAtomId &id);

        void _bindDefaultOwner(const RPZUser &newOwner);

        virtual void _handleAlterationRequest(AlterationPayload &payload) override;
        virtual void _atomsCreated() {};

        virtual RPZAtom* _insertAtom(const RPZAtom &newAtom);
        RPZAtomId _ackSelection(RPZAtom* selectedAtom);
        virtual RPZAtom* _changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner);
        RPZAtomId _removeAtom(RPZAtom* toRemove);
        RPZAtomId _updateAtom(RPZAtom* toUpdate, const AtomUpdates &updates);
        
        virtual void _basicAlterationDone(const QList<RPZAtomId> &updatedIds, const PayloadAlteration &type) {};
        virtual void _updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) {};
        virtual void _updatesDone(const AtomsUpdates &updates) {};
        virtual void _ownerChangeDone(const QList<RPZAtomId> &updatedIds, const RPZUser &newUser) {};

    private:
        mutable QMutex _m_handlingLock;

        //atoms list 
        QHash<RPZAssetHash, int> _assetIdsUsed;
        RPZMap<RPZAtom> _atomsById;

        //credentials handling
        QHash<RPZUserId, QSet<RPZAtomId>> _RPZAtomIdsByOwnerId;
        RPZUser _defaultOwner;

        // redo/undo
        QStack<AlterationPayload> _redoHistory;
        QStack<AlterationPayload> _undoHistory;
        int _payloadHistoryIndex = 0;
        void _registerPayloadForHistory(AlterationPayload &payload);
        AlterationPayload _generateUndoPayload(AlterationPayload &historyPayload);
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
        QPair<int, int> _determineMinMaxLayer(const QList<RPZAtom*> &atoms);
};