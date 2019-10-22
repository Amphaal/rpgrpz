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

class AtomsStorage : public AlterationAcknoledger, public MapDatabase {

    Q_OBJECT

    public:
        AtomsStorage(const AlterationPayload::Source &boundSource);
        
        QVector<RPZAtomId> bufferedSelectedAtomIds() const; //safe
        const AtomsSelectionDescriptor getAtomSelectionDescriptor(const QVector<RPZAtomId> &selectedIds) const; //safe
        
        PossibleActionsOnAtomList getPossibleActions(const QVector<RPZAtomId> &ids);

        const RPZMap<RPZAtom> safe_atoms() const override;
        const QSet<RPZAssetHash> safe_usedAssetsIds() const override;

    public slots:    
        void redo();
        void undo();
        void duplicateAtoms(const QVector<RPZAtomId> &RPZAtomIdList);
        void handleAlterationRequest(AlterationPayload &payload);

    protected:
        const RPZAtom* _getAtomFromId(const RPZAtomId &id);

        virtual void _handleAlterationRequest(AlterationPayload &payload) override;
        virtual void _atomsCreated() {};
        
        virtual void _basicAlterationDone(const QList<RPZAtomId> &updatedIds, const PayloadAlteration &type) {};
        virtual void _updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) {};
        virtual void _updatesDone(const AtomsUpdates &updates) {};

    private:
        mutable QMutex _m_handlingLock;

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
        QPair<int, int> _determineMinMaxLayer(const QList<const RPZAtom*> &atoms);
};