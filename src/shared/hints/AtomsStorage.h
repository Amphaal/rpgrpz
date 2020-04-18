// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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
#include "src/helpers/Authorisations.hpp"

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
        using AtomsAreLeft = bool;

        AtomsStorage(const Payload::Interactor &boundSource);
        
        const AtomsSelectionDescriptor getAtomSelectionDescriptor(const QList<RPZAtom::Id> &selectedIds) const; //safe
        
        PossibleActionsOnAtomList getPossibleActions(const QList<RPZAtom::Id> &ids); //safe

        const ResetPayload generateResetPayload() const; //safe
        AtomsAreLeft restrictPayload(AtomRelatedPayload &payloadToRestrict); //safe

        const QString snapshotSave(const QString &folderToSaveTo); //safe

        const QList<RPZCharacter::UserBound> findUnboundCharacters(const QList<RPZCharacter::UserBound> &availableCharacters); //safe
        
    signals:
        void mapSetup(const RPZMapParameters &mParams, const RPZFogParams &fParams);

    public slots:    
        void redo();
        void undo();
        void duplicateAtoms(const QList<RPZAtom::Id> &idsToDuplicate);
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
        virtual void _fogUpdated(const QList<QPolygonF> &updatedFog) {};

        bool _isAtomOwnable(const RPZAtom &atom) const;
        const QHash<RPZAtom::Id, RPZCharacter::Id>& _ownables() const; //safe
        const QList<RPZAtom::Id> _ownedBy(const RPZCharacter::Id &owner) const; //safe

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

        //sets
        QSet<RPZAtom::Id> _restrictedAtomIds;

        //ownership
        QHash<RPZAtom::Id, RPZCharacter::Id> _ownableAtomIdsByOwner;
        void _syncAtom(const RPZAtom::Id &toUpdate, const RPZAtom::Updates &updates);
        virtual void _atomOwnerChanged(const RPZAtom::Id &target, const RPZCharacter::Id &newOwner) {};

        //duplication
        int _duplicationCount = 0;
        QList<RPZAtom::Id> _latestDuplication;
        RPZMap<RPZAtom> _generateAtomDuplicates(QList<RPZAtom::Id> RPZAtomIdsToDuplicate) const;
        static QPointF _getPositionFromAtomDuplication(const RPZAtom &atomToDuplicate, double distanceFromOriginal);

        //
        QPair<int, int> _determineMinMaxLayer(const QList<const RPZAtom*> &atoms);
};