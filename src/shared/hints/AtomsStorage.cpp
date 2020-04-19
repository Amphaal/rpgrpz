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

#include "AtomsStorage.h"

AtomsStorage::AtomsStorage(const Payload::Interactor &boundSource) : AlterationAcknoledger(boundSource) { }

const ResetPayload AtomsStorage::generateResetPayload() const {
    QMutexLocker l(&_m_handlingLock);
    return ResetPayload(this->_map);
}

MapDatabase& AtomsStorage::map() {
    return this->_map;
}

const MapDatabase& AtomsStorage::map() const {
    return this->_map;
}

const QList<RPZCharacter::UserBound> AtomsStorage::findUnboundCharacters(const QList<RPZCharacter::UserBound> &availableCharacters) {
    QList<RPZCharacter::UserBound> out;

    {
        QMutexLocker l(&_m_handlingLock);

        for (auto const &bond : availableCharacters) {
            auto charId = bond.second.id();
            if (!charId) continue;

            auto boundAtomId = this->_ownableAtomIdsByOwner.key(charId);
            if (boundAtomId) continue;  // is bound, skip

            out += bond;
        }
    }

    return out;
}

void AtomsStorage::_replaceMap(const MapDatabase &map) {
    QMutexLocker l(&_m_handlingLock);
    this->_map = map;
}

const QString AtomsStorage::snapshotSave(const QString &folderToSaveTo) {
    QMutexLocker l(&_m_handlingLock);
    return this->_map.snapshotSave(folderToSaveTo);
}

PossibleActionsOnAtomList AtomsStorage::getPossibleActions(const QList<RPZAtom::Id> &ids) {
    QMutexLocker l(&_m_handlingLock);
    PossibleActionsOnAtomList out;

    // no actions possible if not host able
    if (!Authorisations::isHostAble()) return out;

    // availability
    auto areIdsSelected = !ids.isEmpty();
    out.canChangeAvailability = areIdsSelected;

    // redo/undo
    out.somethingRedoable = this->_canRedo() > -1;
    out.somethingUndoable = this->_canUndo() > -1;

    auto containsAnInteractiveAtom = false;
    auto cannotBeManuallyHidden = false;

    // iterate
    QList<const RPZAtom*> atomList;
    for (const auto &id : ids) {
        // get atom
        auto atom = this->_map.atomPtr(id);

        if (atom->category() == RPZAtom::Category::Interactive) containsAnInteractiveAtom = true;
        if (!atom->canBeManuallyHidden()) cannotBeManuallyHidden = true;

        if (!atom) continue;

        atomList += atom;

        // if is locked, break
        if (atom->isLocked()) {
            return out;
        }
    }

    // else, activate most
    out.canChangeLayer = areIdsSelected && !containsAnInteractiveAtom;
    out.canCopy = areIdsSelected;
    out.canChangeVisibility = areIdsSelected && !cannotBeManuallyHidden;
    out.canRemove = areIdsSelected;

    // determine min/max
    if (out.canChangeLayer) {
        auto minMaxLayer = this->_determineMinMaxLayer(atomList);
        out.targetDownLayer = minMaxLayer.first;
        out.targetUpLayer = minMaxLayer.second;
    }

    return out;
}

QPair<int, int> AtomsStorage::_determineMinMaxLayer(const QList<const RPZAtom*> &atoms) {
    // targets
    auto firstPass = true;
    auto riseLayoutTarget = 0;
    auto lowerLayoutTarget = 0;

    for (const auto atom : atoms) {
        auto layer = atom->layer();

        if (firstPass) {
            firstPass = false;
            riseLayoutTarget = layer;
            lowerLayoutTarget = layer;
            continue;
        }

        if (layer > riseLayoutTarget) riseLayoutTarget = layer;
        if (layer < lowerLayoutTarget) lowerLayoutTarget = layer;
    }

    riseLayoutTarget++;
    lowerLayoutTarget--;

    return QPair<int, int>(lowerLayoutTarget, riseLayoutTarget);
}

const AtomsSelectionDescriptor AtomsStorage::getAtomSelectionDescriptor(const QList<RPZAtom::Id> &selectedIds) const {
    AtomsSelectionDescriptor out;

    out.selectedAtomIds = selectedIds;
    auto takeFirstAtomAsTemplate = selectedIds.count() == 1;

    {
        QMutexLocker m(&this->_m_handlingLock);

        for (const auto id : selectedIds) {
            auto atom = this->_map.atom(id);
            if (atom.isEmpty()) continue;

            out.representedTypes.insert(atom.type());

            if (takeFirstAtomAsTemplate) out.templateAtom = atom;
        }
    }

    return out;
}

AtomsStorage::AtomsAreLeft AtomsStorage::restrictPayload(AtomRelatedPayload &payloadToRestrict) {
    // no need to touch
    if (auto hasRestrictions = this->_restrictedAtomIds.count(); !hasRestrictions) {
        return true;
    }

    // restrict
    return payloadToRestrict.restrictTargetedAtoms(this->_restrictedAtomIds);
}

/////////////
// HISTORY //
/////////////

void AtomsStorage::_registerPayloadForHistory(const AlterationPayload &payload) {
    // do not register again if payload is already from timeline
    if (payload.isFromTimeline()) return;

    // do nothing if payload is not allowed to be registered
    if (!payload.undoRedoAllowed()) return;

    // cut branch
    while (this->_payloadHistoryIndex) {
        this->_undoHistory.pop();
        this->_redoHistory.pop();
        this->_payloadHistoryIndex--;
    }

    // store redo
    this->_redoHistory.push(payload);

    // store undo
    auto correspondingUndo = this->_generateUndoPayload(payload);
    this->_undoHistory.push(correspondingUndo);
}

int AtomsStorage::_canRedo() {
    // if already on the most recent, abort
    if (!this->_payloadHistoryIndex) return -1;

    // if no history, abort
    auto count = this->_redoHistory.count();
    if (!count) return -1;

    // check if targeted payload exists
    auto toReach = this->_payloadHistoryIndex - 1;
    auto toReachIndex = (count - toReach) - 1;

    return toReachIndex;
}

int AtomsStorage::_canUndo() {
    // if no history, abort
    auto count = this->_undoHistory.count();
    if (!count) return -1;

    // check if targeted payload exists
    auto toReach = this->_payloadHistoryIndex + 1;
    auto toReachIndex = count - toReach;

    return toReachIndex;
}

void AtomsStorage::undo() {
    auto toReachIndex = this->_canUndo();
    if (toReachIndex < 0) return;

    // get stored payload and handle it
    auto st_payload = this->_undoHistory.at(toReachIndex);
    st_payload.tagAsFromTimeline();

    // update the index
    this->_payloadHistoryIndex++;

    // propagate
    AlterationHandler::get()->queueAlteration(this, st_payload);
}

void AtomsStorage::redo() {
     auto toReachIndex = this->_canRedo();
    if (toReachIndex < 0) return;

    // get stored payload and handle it
    auto st_payload = this->_redoHistory.at(toReachIndex);
    st_payload.tagAsFromTimeline();

    // update the index
    this->_payloadHistoryIndex--;

    // process
    AlterationHandler::get()->queueAlteration(this, st_payload);
}

AlterationPayload AtomsStorage::_generateUndoPayload(const AlterationPayload &fromHistoryPayload) {
    switch (auto type = fromHistoryPayload.type()) {
        case Payload::Alteration::BulkMetadataChanged: {
            auto casted = (BulkMetadataChangedPayload*)&fromHistoryPayload;
            auto intialAtoms = casted->atomsUpdates();

            RPZAtom::ManyUpdates out;
            for (auto i = intialAtoms.constBegin(); i != intialAtoms.constEnd(); i++) {
                // get atom
                auto snowflakeId = i.key();
                auto atom = this->_map.atomPtr(snowflakeId);
                if (!atom) continue;

                // init hash with old values
                RPZAtom::Updates oldValues;
                for (const auto &param : i.value().keys()) {
                    auto oldValue = atom->metadata(param);
                    oldValues.insert(param, oldValue);
                }

                out.insert(snowflakeId, oldValues);
            }

            return BulkMetadataChangedPayload(out);
        }
        break;

        case Payload::Alteration::MetadataChanged: {
            auto casted = (MetadataChangedPayload*)&fromHistoryPayload;

            RPZAtom::ManyUpdates out;
            for (const auto &id : casted->targetRPZAtomIds()) {
                // get atom
                auto atom = this->_map.atomPtr(id);
                if (!atom) continue;

                RPZAtom::Updates oldValues;
                for (const auto &param : casted->updates().keys()) {
                    auto oldValue = atom->metadata(param);
                    oldValues.insert(param, oldValue);
                }

                out.insert(id, oldValues);
            }

            return BulkMetadataChangedPayload(out);
        }
        break;

        case Payload::Alteration::Added: {
            auto casted = (AddedPayload*)&fromHistoryPayload;
            return RemovedPayload(casted->atoms().keys());
        }
        break;

        case Payload::Alteration::Removed: {
            auto casted = (RemovedPayload*)&fromHistoryPayload;

            QList<RPZAtom> out;
            for (const auto &id : casted->targetRPZAtomIds()) {
                auto atom = this->_map.atom(id);
                if (atom.isEmpty()) continue;

                out += atom;
            }

            return AddedPayload(out);
        }
        break;

        default:
            qWarning() << qUtf8Printable(QStringLiteral(u"This payload type %1 is not handled to be reverted !").arg((int)type));
            break;
    }

    return fromHistoryPayload;
}


/////////////////
// END HISTORY //
/////////////////

//////////////
/* ELEMENTS */
//////////////

void AtomsStorage::handleAlterationRequest(const AlterationPayload &payload) {
    return this->_handleAlterationRequest(payload);
}

void AtomsStorage::_handleAlterationRequest(const AlterationPayload &payload) {
    QMutexLocker lock(&this->_m_handlingLock);

    auto pType = payload.type();

    // may register for history
    this->_registerPayloadForHistory(payload);

    // on reset
    if (auto mPayload = dynamic_cast<const ResetPayload*>(&payload)) {
        // clear lists
        this->_undoHistory.clear();
        this->_redoHistory.clear();
        this->_restrictedAtomIds.clear();
        this->_map.clear();
        this->_ownableAtomIdsByOwner.clear();

        // set new map params
        auto mParams = mPayload->mapParameters();
        this->_map.setMapParams(mParams);

        // set fog params
        auto fParams = mPayload->fogParameters();
        this->_map.setFogParams(fParams);

        lock.unlock();
            emit mapSetup(mParams, fParams);
        lock.relock();
    }

    // reset/insert types
    if (auto mPayload = dynamic_cast<const AtomsWielderPayload*>(&payload)) {
        QList<RPZAtom::Id> insertedIds;

        auto atoms = mPayload->atoms();
        for (auto i = atoms.begin(); i != atoms.end(); i++) {
            // add to Db
            auto &id = i.key();
            auto &atom = i.value();

            this->_map.addAtom(atom);

            // add to restricted
            if (atom.isRestrictedAtom()) {
                this->_restrictedAtomIds += id;
            }

            // add to ownable
            if (this->_isAtomOwnable(atom)) {
                auto characterId = atom.characterId();
                this->_ownableAtomIdsByOwner.insert(id, characterId);
                this->_atomOwnerChanged(id, characterId);
            }

            // handler for inheritors
            this->_atomAdded(atom);

            insertedIds += id;
        }

        this->_basicAlterationDone(insertedIds, pType);
    } else if (auto mPayload = dynamic_cast<const BulkMetadataChangedPayload*>(&payload)) {  // bulk
        auto updatesById = mPayload->atomsUpdates();

        for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
            auto const &id = i.key();
            auto const &updates = i.value();

            auto atom = this->_map.atom(id);
            if (atom.isEmpty()) continue;

            this->_syncAtom(id, updates);
        }

        this->_updatesDone(updatesById);
    } else if (auto mPayload = dynamic_cast<const MultipleAtomTargetsPayload*>(&payload)) {  // multi target format
        QList<RPZAtom::Id> alteredIds;
        RPZAtom::Updates maybeUpdates;

        if (auto nPayload = dynamic_cast<const MetadataChangedPayload*>(&payload)) {
            maybeUpdates = nPayload->updates();
        }

        for (const auto &id : mPayload->targetRPZAtomIds()) {
            auto atom = this->_map.atom(id);
            if (atom.isEmpty()) continue;

            if (pType == Payload::Alteration::Removed) {
                this->_map.removeAtom(id);  // remove from db
                this->_restrictedAtomIds.remove(id);  // remove from restricted
                this->_ownableAtomIdsByOwner.remove(id);
            }

            if (pType == Payload::Alteration::MetadataChanged) {
                this->_syncAtom(id, maybeUpdates);
            }

            alteredIds += id;
        }

        if (!maybeUpdates.isEmpty()) this->_updatesDone(alteredIds, maybeUpdates);
        else
            this->_basicAlterationDone(alteredIds, pType);

    } else if (auto mPayload = dynamic_cast<const FogChangedPayload*>(&payload)) {  // fog update
        auto newFog = this->_map.alterFog(*mPayload);
        this->_fogUpdated(newFog);
    } else if (auto mPayload = dynamic_cast<const FogModeChangedPayload*>(&payload)) {  // fog mode change
        this->_map.changeFogMode(mPayload->mode());
    }
}

const QHash<RPZAtom::Id, RPZCharacter::Id>& AtomsStorage::_ownables() const {
    QMutexLocker l(&this->_m_handlingLock);
    return this->_ownableAtomIdsByOwner;
}

const QList<RPZAtom::Id> AtomsStorage::_ownedBy(const RPZCharacter::Id &owner) const {
    QMutexLocker l(&this->_m_handlingLock);
    return this->_ownableAtomIdsByOwner.keys(owner);
}

void AtomsStorage::_syncAtom(const RPZAtom::Id &toUpdate, const RPZAtom::Updates &updates) {
    // update db
    this->_map.updateAtom(toUpdate, updates);

    // check if character id update
    if (!updates.contains(RPZAtom::Parameter::CharacterId)) return;

    auto updatedCharId = updates.value(RPZAtom::Parameter::CharacterId).toULongLong();
    auto boundCharId = this->_ownableAtomIdsByOwner.value(toUpdate);

    this->_ownableAtomIdsByOwner.insert(toUpdate, updatedCharId);

    if (updatedCharId != boundCharId) {
        this->_atomOwnerChanged(toUpdate, updatedCharId);
    }
}

//
//
//

void AtomsStorage::duplicateAtoms(const QList<RPZAtom::Id> &idsToDuplicate) {
    // check if a recent duplication have been made, and if it was about the same atoms
    if (this->_latestDuplication != idsToDuplicate) {  // if not
        // reset duplication cache
        this->_latestDuplication = idsToDuplicate;
        this->_duplicationCount = 1;
    } else {
        // else, increment subsequent duplication count
        this->_duplicationCount++;
    }

    // generate duplicated atoms
    auto newAtoms = this->_generateAtomDuplicates(idsToDuplicate);
    if (!newAtoms.count()) return;

    // request insertion
    AddedPayload added(newAtoms.values());
    AlterationHandler::get()->queueAlteration(this, added);

    // request selection
    SelectedPayload selected(newAtoms.keys());
    AlterationHandler::get()->queueAlteration(this, selected);
}

RPZMap<RPZAtom> AtomsStorage::_generateAtomDuplicates(QList<RPZAtom::Id> RPZAtomIdsToDuplicate) const {
    RPZMap<RPZAtom> newAtoms;

    // distance from original
    auto distFromOrigin = this->_map.mapParams().tileWidthInPoints() * this->_duplicationCount;

    // sort for ordered id generation
    std::sort(RPZAtomIdsToDuplicate.begin(), RPZAtomIdsToDuplicate.end());

    // create the new atoms from the selection
    for (const auto &atomId : RPZAtomIdsToDuplicate) {
        // skip if RPZAtom::Id does not exist
        auto atom = this->_map.atom(atomId);
        if (atom.isEmpty()) continue;

        // check if is copyable
        if (!atom.isCopyable()) continue;

        // create copy atom, update its id
        RPZAtom newAtom(atom);
        newAtom.shuffleId();

        // find new position for the duplicated atom
        auto newPos = _getPositionFromAtomDuplication(newAtom, distFromOrigin);
        newAtom.setMetadata(RPZAtom::Parameter::Position, newPos);

        // adds it to the final list
        newAtoms.insert(newAtom.id(), newAtom);
    }

    return newAtoms;
}

bool AtomsStorage::_isAtomOwnable(const RPZAtom &atom) const {
    return atom.type() == RPZAtom::Type::Player;
}

QPointF AtomsStorage::_getPositionFromAtomDuplication(const RPZAtom &atomToDuplicate, double distanceFromOriginal) {
    auto currPos = atomToDuplicate.pos();
    currPos.setX(currPos.x() + distanceFromOriginal);
    currPos.setY(currPos.y() + distanceFromOriginal);
    return currPos;
}


//////////////////
/* END ELEMENTS */
//////////////////
