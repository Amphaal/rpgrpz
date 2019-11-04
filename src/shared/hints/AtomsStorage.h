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

#include "src/network/rpz/client/RPZClient.h"

struct AtomsSelectionDescriptor {
    QSet<RPZAtom::Type> representedTypes;
    RPZAtom templateAtom;
    QList<RPZAtom::Id> selectedAtomIds;
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
        typedef bool AtomsAreLeft;

        AtomsStorage(const Payload::Source &boundSource);
        
        QList<RPZAtom::Id> bufferedSelectedAtomIds() const; //safe
        const AtomsSelectionDescriptor getAtomSelectionDescriptor(const QList<RPZAtom::Id> &selectedIds) const; //safe
        
        PossibleActionsOnAtomList getPossibleActions(const QList<RPZAtom::Id> &ids);

        const ResetPayload generateResetPayload() const;
        AtomsStorage::AtomsAreLeft restrictPayload(AtomRelatedPayload &payloadToRestrict);

        const QString snapshotSave(const QString &folderToSaveTo); //safe

    public slots:    
        void redo();
        void undo();
        void duplicateAtoms(const QList<RPZAtom::Id> &RPZAtomIdList);
        void handleAlterationRequest(const AlterationPayload &payload);

    protected:
        MapDatabase& map();
        const MapDatabase& map() const;
        void _replaceMap(const MapDatabase &map);

        virtual void _handleAlterationRequest(const AlterationPayload &payload) override;
        virtual void _atomAdded(const RPZAtom &added) {};

        virtual void _basicAlterationDone(const QList<RPZAtom::Id> &updatedIds, const Payload::Alteration &type) {};
        virtual void _updatesDone(const QList<RPZAtom::Id> &updatedIds, const RPZAtom::Updates &updates) {};
        virtual void _updatesDone(const RPZAtom::ManyUpdates &updates) {};

    private:
        mutable QMutex _m_handlingLock;
        MapDatabase _map;

        // redo/undo
        QStack<AlterationPayload> _redoHistory;
        QStack<AlterationPayload> _undoHistory;
        int _payloadHistoryIndex = 0;
        void _registerPayloadForHistory(const AlterationPayload &payload);
        AlterationPayload _generateUndoPayload(const AlterationPayload &historyPayload);
        int _canRedo();
        int _canUndo();

        //selected
        QSet<RPZAtom::Id> _selectedRPZAtomIds;
        QSet<RPZAtom::Id> _restrictedAtomIds;

        //duplication
        int _duplicationCount = 0;
        QList<RPZAtom::Id> _latestDuplication;
        RPZMap<RPZAtom> _generateAtomDuplicates(const QList<RPZAtom::Id> &RPZAtomIdsToDuplicate) const;
        static constexpr int _pixelStepPosDuplication = 10;
        static QPointF _getPositionFromAtomDuplication(const RPZAtom &atomToDuplicate, int duplicateCount);

        //
        QPair<int, int> _determineMinMaxLayer(const QList<const RPZAtom*> &atoms);
};