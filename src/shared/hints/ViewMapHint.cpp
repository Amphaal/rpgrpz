// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include "ViewMapHint.h"

ViewMapHint::ViewMapHint() : AtomsStorage(Payload::Interactor::Local_Map) {
    // default layer from settings
    this->setDefaultLayer(AppContext::settings()->defaultLayer());
    this->setDefaultVisibility((int)AppContext::settings()->hiddenAtomAsDefault());
}

const RPZAtom ViewMapHint::templateAtom() const {
    QMutexLocker m(&this->_m_templateAtom);
    return this->_templateAtom;
}

QGraphicsItem* ViewMapHint::ghostItem() const {
    QMutexLocker m(&this->_m_ghostItem);
    return this->_ghostItem;
}

MapViewFog* ViewMapHint::fogItem() const {
    QMutexLocker l(&this->_m_fogItem);
    return this->_fogItem;
}

const QList<QGraphicsItem*> ViewMapHint::_gis(const QList<RPZAtom::Id> &atomIds) const {
    QList<QGraphicsItem*> out;

    for (const auto &id : atomIds) {
        auto gi = this->_GItemsById.value(id);
        if (!gi) continue;
        out += gi;
    }

    return out;
}

void ViewMapHint::_atomOwnerChanged(const RPZAtom::Id &target, const RPZCharacter::Id &newOwner) {
    // check an item exists
    auto gi = this->_GItemsById.value(target);
    if (!gi) return;

    // do the user own this atom ?
    auto owns = Authorisations::isHostAble() || (this->_myCharacterId && this->_myCharacterId == newOwner);

    // if so, add to owned list
    if (owns) this->_ownedTokenIds.insert(target);
    else  // if not, remove from it
        this->_ownedTokenIds.remove(target);

    emit changedOwnership({gi}, owns);
}

bool ViewMapHint::_hasOwnershipOf(const RPZAtom &atom) const {
    if (Authorisations::isHostAble()) return true;
    if (!this->_isAtomOwnable(atom)) return false;

    auto id = atom.id();
    auto has = this->_ownedTokenIds.contains(id);

    return has;
}

void ViewMapHint::_checkForOwnedTokens() {
    auto isHostAble = Authorisations::isHostAble();
    if (!isHostAble && !this->_myCharacterId) return;

    QList<QGraphicsItem*> ownedGIs;
    QList<QGraphicsItem*> notOwnedGIs;

    QSet<RPZAtom::Id> nowOwned;
    if (isHostAble) {  // if is host, owns everything
        auto oks = this->_ownables().keys();
        nowOwned = QSet<RPZAtom::Id>(oks.begin(), oks.end());
    } else {  // if not, owns from specified character to impersonate
        auto oks = this->_ownables().keys(this->_myCharacterId);
        nowOwned = QSet<RPZAtom::Id>(oks.begin(), oks.end());
    }

    // update owned tokens
    QSet<RPZAtom::Id> previouslyOwned = this->_ownedTokenIds;
    this->_ownedTokenIds = nowOwned;

    // find updatables
    auto notOwnedAnymore = previouslyOwned.subtract(nowOwned);
    auto newlyOwned = nowOwned.subtract(previouslyOwned);

    // find associated graphics items
    ownedGIs = this->_gis(newlyOwned.values());
    notOwnedGIs = this->_gis(notOwnedAnymore.values());

    // signals
    if (ownedGIs.count()) {
        emit changedOwnership(ownedGIs, true);
    }
    if (notOwnedGIs.count()) {
        emit changedOwnership(notOwnedGIs, false);
    }
}

void ViewMapHint::mightUpdateOwnedTokens(const RPZUser &owner) {
    // check if has character
    auto character = owner.character();
    auto charId = character.id();
    if (!charId) return;

    // check if has ownings
    auto ownings = this->_ownedBy(charId);
    if (!ownings.count()) return;

    // send payload
    MetadataChangedPayload payload(ownings, {
        { RPZAtom::Parameter::CharacterName, character.toString() },
        { RPZAtom::Parameter::DefaultPlayerColor, owner.color() }
    });
    AlterationHandler::get()->queueAlteration(this, payload);
}

void ViewMapHint::defineImpersonatingCharacter(const RPZCharacter::Id &toImpersonate) {
    QMutexLocker l(&this->_m_GItemsById);
    this->_myCharacterId = toImpersonate;
}

void ViewMapHint::_updateTemplateAtom(RPZAtom::Updates updates) {
    // update template
    {
        QMutexLocker m(&this->_m_templateAtom);
        this->_templateAtom.setMetadata(updates);
    }

    // get ghost item ptr
    this->_m_ghostItem.lock();
    auto ghostItem = this->_ghostItem;
    this->_m_ghostItem.unlock();

    // if no ghost, skip
    if (!ghostItem) return;

    // remove illegal parameters from updates
    auto templateLegals = this->_templateAtom.legalParameters();
    auto illegalsList = updates.keys();
    auto illegals = QSet<RPZAtom::Parameter>(illegalsList.begin(), illegalsList.end()).subtract(templateLegals);
    for (const auto &illegal : illegals) updates.remove(illegal);

    // if no more updates, skip
    if (!updates.count()) return;

    // update ghost
    emit requestingUIUpdate({ghostItem}, updates, true);
}

void ViewMapHint::setDefaultLayer(int layer) {
    this->_updateTemplateAtom({{RPZAtom::Parameter::Layer, layer}});
}

void ViewMapHint::setDefaultVisibility(int checkboxState) {
    this->_updateTemplateAtom({{RPZAtom::Parameter::Hidden, (bool)checkboxState}});
}

void ViewMapHint::notifyFocusedItem(QGraphicsItem* focusedItem) {
    // extract id
    auto id = this->getAtomIdFromGraphicsItem(focusedItem);
    if (!id) return;

    // send payload
    FocusedPayload payload(id);
    AlterationHandler::get()->queueAlteration(this, payload);
}


void ViewMapHint::mightNotifyMovement(const QList<QGraphicsItem*> &itemsWhoMightHaveMoved) {
    // generate args for payload
    RPZAtom::ManyUpdates coords;
    for (const auto gi : itemsWhoMightHaveMoved) {
        // find id
        auto id = this->getAtomIdFromGraphicsItem(gi);
        if (!id) continue;

        // find corresponding atom
        auto cAtom = this->map().atom(id);
        if (cAtom.isEmpty()) continue;

        // pos have not changed
        auto newPos = gi->pos();
        auto oldPos = cAtom.pos();
        if (oldPos == newPos) continue;

        // add update into alteration
        RPZAtom::Updates updates {{ RPZAtom::Parameter::Position, newPos }};
        coords.insert(cAtom.id(), updates);
    }

    // if nothing happened
    if (!coords.count()) return;

    // inform moving
    BulkMetadataChangedPayload payload(coords);
    AlterationHandler::get()->queueAlteration(this, payload);
}

void ViewMapHint::notifySelectedItems(const QList<QGraphicsItem*> &selectedItems) {
    auto ids = this->getAtomIdsFromGraphicsItems(selectedItems);  // allow selecting nothing
    SelectedPayload payload(ids);
    AlterationHandler::get()->queueAlteration(this, payload);
}

/////////////////////////////
// Atom insertion helpers //
/////////////////////////////


void ViewMapHint::_resetTemplate(const RPZToy &from) {
    if (!from.isEmpty()) {
        QMutexLocker m(&this->_m_templateAtom);

        // update atom template
        this->_templateAtom.changeType(from.atomType());
        this->_templateAtom.setShape(from.shapeRect());
        this->_templateAtom.setMetadata(RPZAtom::Parameter::ShapeCenter, from.shapeCenter());
        this->_templateAtom.setMetadata(RPZAtom::Parameter::AssetHash, from.hash());
        this->_templateAtom.setMetadata(RPZAtom::Parameter::AssetName, from.name());
    }

    // update toy template
    QMutexLocker l2(&this->_m_templateToy);
    this->_templateToy = from;
}

QGraphicsItem* ViewMapHint::_generateGhostItem(const RPZToy &toy, QGraphicsItem* &oldGhostToDelete) {
    // reset values from template
    this->_resetTemplate(toy);

    {
        QMutexLocker m(&this->_m_ghostItem);

        // request ghost deletion from scene if existing
        if (this->_ghostItem) {
            oldGhostToDelete = this->_ghostItem;
            this->_ghostItem = nullptr;
        }

        // generate for scene addition
        this->_ghostItem = generateGraphicsFromTemplate(true);
        return this->_ghostItem;
    }
}

QGraphicsItem* ViewMapHint::generateGraphicsFromTemplate(bool hiddenAsDefault) const {
    QMutexLocker l1(&this->_m_templateAtom);
    QMutexLocker l2(&this->_m_templateToy);

    // not based on template toy, skip
    if (this->_templateToy.isEmpty()) return nullptr;

    // generate
    auto item = AtomRenderer::createGraphicsItem(
        this->_templateAtom,
        this->_templateToy,
        true,
        false
    );

    // auto visibility
    item->setVisible(!hiddenAsDefault);

    return item;
}

RPZAtom::Id ViewMapHint::integrateGraphicsItemAsPayload(QGraphicsItem* graphicsItem) const {
    if (!graphicsItem) return 0;

    // from ghost item / temporary drawing
    RPZAtom newAtom;
    {
        QMutexLocker l1(&this->_m_templateAtom);
        newAtom = AtomConverter::cloneAtomTemplateFromGraphics(graphicsItem, this->_templateAtom);
    }

    // queue
    AddedPayload payload(newAtom);
    AlterationHandler::get()->queueAlteration(this, payload);

    return newAtom.id();
}

void ViewMapHint::notifyWalk(const QHash<QGraphicsItem*, QPointF> &toWalk) {
    RPZAtom::ManyUpdates updates;
    for (auto i = toWalk.begin(); i != toWalk.end(); i++) {
        auto atomId = this->getAtomIdFromGraphicsItem(i.key());
        updates.insert(atomId, {{RPZAtom::Parameter::Position, i.value()}});
    }

    if (!updates.count()) return;

    BulkMetadataChangedPayload payload(updates);
    AlterationHandler::get()->queueAlteration(this, payload);
}

/////////////////////////////////
// END Atom insertion helpers //
/////////////////////////////////

/////////////////////////
// Integration handler //
/////////////////////////


QGraphicsItem* ViewMapHint::_buildGraphicsItemFromAtom(const RPZAtom &atomToBuildFrom) {
    QGraphicsItem* newItem = nullptr;
    auto assetHash = atomToBuildFrom.assetHash();
    auto asset = AssetsDatabase::get()->asset(atomToBuildFrom.assetHash());
    auto hasMissingAsset = !assetHash.isEmpty() && !asset;

    // atom links to missing asset from DB
    if (hasMissingAsset) {
        // add placeholder
        auto placeholder = AtomRenderer::createMissingAssetPlaceholderItem(atomToBuildFrom);
        newItem = placeholder;

        // add graphic item to list of items to replace at times
        this->_missingAssetHashesFromDb.insert(assetHash, placeholder);
    } else {  // default
        newItem = AtomRenderer::createGraphicsItem(
            atomToBuildFrom,
            *asset,
            false,
            this->_hasOwnershipOf(atomToBuildFrom)
        );
    }

    // save pointer ref
    this->_crossBindingAtomWithGI(
        atomToBuildFrom,
        newItem
    );

    return newItem;
}

void ViewMapHint::_replaceMissingAssetPlaceholders(const RPZAsset &asset) {
    OrderedGraphicsItems newGis;
    QSet<QGraphicsItem*> setOfGraphicsItemsToReplace;

    auto hash = asset.hash();
    auto pathToFile = asset.filepath();

    if (!this->_missingAssetHashesFromDb.contains(hash)) return;  // no assetHash, skip
    if (pathToFile.isNull()) return;  // path to file empty, skip

    // get uniques ids
    auto setOfGraphicsItemsToReplaceList = this->_missingAssetHashesFromDb.values(hash);
    setOfGraphicsItemsToReplace = QSet<QGraphicsItem*>(setOfGraphicsItemsToReplaceList.begin(), setOfGraphicsItemsToReplaceList.end());

    // iterate through the list of GI to replace
    for (const auto itemToReplace : setOfGraphicsItemsToReplace) {
        // find id
        auto id = this->getAtomIdFromGraphicsItem(itemToReplace);
        if (!id) continue;

        // find corresponding atom
        auto atom = this->map().atom(id);
        if (atom.isEmpty()) continue;

        // create the new graphics item
        auto newGi = AtomRenderer::createGraphicsItem(
            atom,
            asset,
            false,
            this->_hasOwnershipOf(atom)
        );

        this->_crossBindingAtomWithGI(atom, newGi);
        RPZQVariant::setGraphicsItemToReplace(newGi, itemToReplace);
        newGis.insert(id, newGi);
    }

    // clear the id from the missing list
    this->_missingAssetHashesFromDb.remove(hash);

    // replace by new
    emit requestingUIAlteration(Payload::Alteration::Replaced, newGis);
}

void ViewMapHint::handlePreviewRequest(const AtomsSelectionDescriptor &selectionDescriptor, const RPZAtom::Parameter &parameter, const QVariant &value) {
    // create updates container
    RPZAtom::Updates updates;
    updates.insert(parameter, value);

    QList<QGraphicsItem*> toUpdate;

    // is ghost that must be targeted
    if (!selectionDescriptor.selectedAtomIds.count() && !selectionDescriptor.templateAtom.isEmpty()) {
        QMutexLocker l(&this->_m_ghostItem);

        if (this->_ghostItem) toUpdate += this->_ghostItem;
    } else {  // selected atoms to change
        QMutexLocker l(&this->_m_GItemsById);

        for (const auto &id : selectionDescriptor.selectedAtomIds) {
            toUpdate += this->_GItemsById.value(id);
        }
    }

    emit requestingUIUpdate(toUpdate, updates, true);
}


/////////////////////////////
// END Integration handler //
/////////////////////////////

//////////////////////
// Internal Helpers //
//////////////////////

void ViewMapHint::_crossBindingAtomWithGI(const RPZAtom &atom, QGraphicsItem* gi) {
    auto id = atom.id();
    this->_GItemsById.insert(id, gi);
    RPZQVariant::setAtomId(gi, id);
}

const QList<RPZAtom::Id> ViewMapHint::getAtomIdsFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const {
    QList<RPZAtom::Id> list;

    for (const auto e : listToFetch) {
        auto id = this->getAtomIdFromGraphicsItem(e);
        if (id) list += id;
    }

    return list;
}

RPZAtom::Id ViewMapHint::getAtomIdFromGraphicsItem(const QGraphicsItem* toFetch) const {
    if (!toFetch) {
        qWarning() << "Cannot fetch Atom Id from this non-existant GraphicsItem...";
        return 0;
    }

    auto id = RPZQVariant::atomId(toFetch);
    if (!id) qWarning() << "No atom assigned to this graphics item...";

    return id;
}

//////////////////////////
// END Internal Helpers //
//////////////////////////

/////////////////////////////
// AtomsStorage Overriding //
/////////////////////////////

// alter Scene
void ViewMapHint::_handleAlterationRequest(const AlterationPayload &payload) {
    QMutexLocker l(&this->_m_GItemsById);

    // if reset (before)
    if (auto mPayload = dynamic_cast<const ResetPayload*>(&payload)) {
        // clear cache of missing qGraphicsItem
        this->_missingAssetHashesFromDb.clear();

        // params
        auto mParams = mPayload->mapParameters();
        auto fParams = mPayload->fogParameters();

        // parameterize atom renderer
        AtomRenderer::defineMapParameters(mParams);

        // clear GI lists
        this->_GItemsById.clear();
        this->_ownedTokenIds.clear();

        // delete ghost
        this->_m_ghostItem.lock();
            this->_ghostItem = nullptr;
        this->_m_ghostItem.unlock();

        // reset fog
        {
            QMutexLocker l(&this->_m_fogItem);
            this->_fogItem = new MapViewFog(fParams, mParams);  // do not delete, the old one will be in UI thread !
        }

        // reset descriptor
        emit atomDescriptorUpdated();

        // reset ssi
        this->_singleSelectionInteractible = SingleSelectionInteractible();
    }

    // standard handling
    AtomsStorage::_handleAlterationRequest(payload);

    // if selected, analyse selection
    if (auto mPayload = dynamic_cast<const SelectedPayload*>(&payload)) {
        auto ssi = _generateSSI(mPayload);
        emit atomDescriptorUpdated(ssi.interactible);

        this->_singleSelectionInteractible = ssi;

    } else if (auto mPayload = dynamic_cast<const FogModeChangedPayload*>(&payload)) {  // fog mode
        emit fogModeChanged(mPayload->mode());

    } else if (dynamic_cast<const ResetPayload*>(&payload)) {  // if reset (afterward)
        // bind owned tokens
        this->_checkForOwnedTokens();

    } else if (auto mPayload = dynamic_cast<const AssetChangedPayload*>(&payload)) {  // if asset changed
        this->_replaceMissingAssetPlaceholders(mPayload->assetMetadata());

    } else if (auto mPayload = dynamic_cast<const ToySelectedPayload*>(&payload)) {  // if asset selected
        // generate ghost
        QGraphicsItem* maybeToDelete = nullptr;
        QGraphicsItem* newGhost = nullptr;

        auto toy = mPayload->selectedToy();
        newGhost = this->_generateGhostItem(toy, maybeToDelete);

        // request deletion previous ghost
        if (maybeToDelete) emit requestingUIAlteration(Payload::Alteration::Removed, {maybeToDelete});

        // request addition of new ghost
        emit requestingUIAlteration(Payload::Alteration::ToySelected, {newGhost});

    } else if (auto mPayload = dynamic_cast<const BulkMetadataChangedPayload*>(&payload)) {  // if updated / deleted
        this->_mightUpdateAtomDescriptor(mPayload->atomsUpdates().keys());

    } else if (auto mPayload = dynamic_cast<const MultipleAtomTargetsPayload*>(&payload)) {
        this->_mightUpdateAtomDescriptor(mPayload->targetRPZAtomIds());

    } else if (auto mPayload = dynamic_cast<const RemovedPayload*>(&payload)) {
        this->_mightUpdateAtomDescriptor(mPayload->targetRPZAtomIds());

    } else if (auto mPayload = dynamic_cast<const AtomTemplateChangedPayload*>(&payload)) {  // if template changed
        this->_updateTemplateAtom(mPayload->updates());
    }
}

void  ViewMapHint::_fogUpdated(const QList<QPolygonF> &updatedFog) {
    emit fogChanged(updatedFog);
}

void ViewMapHint::_mightUpdateAtomDescriptor(const QList<RPZAtom::Id> &idsUpdated) {
    // if no single interactive selection, no need to update
    if (!this->_singleSelectionInteractible.isInteractive) return;

    // if list of updates does not contain interactible, skip
    auto interactibleId = this->_singleSelectionInteractible.interactible.id();
    if (!idsUpdated.contains(interactibleId)) return;

    auto atomPtr = this->map().atomPtr(interactibleId);

    // deleted, update with empty
    if (!atomPtr) {
        this->_singleSelectionInteractible = SingleSelectionInteractible();

        emit atomDescriptorUpdated();
    } else {  // existing, update descriptor
        emit atomDescriptorUpdated(*atomPtr);
    }
}

const ViewMapHint::SingleSelectionInteractible ViewMapHint::_generateSSI(const SelectedPayload* payload) const {
    SingleSelectionInteractible out;

    // not appliable if multiple
    auto targets = payload->targetRPZAtomIds();
    if (targets.count() != 1) return out;

    // not appliable if atom is not interactive
    auto id = targets.first();
    auto atom = this->map().atom(id);
    if (atom.category() != RPZAtom::Category::Interactive) return out;

    // set values
    out.interactible = atom;
    out.isInteractive = true;

    return out;
}

void ViewMapHint::_atomAdded(const RPZAtom &added) {
    this->_buildGraphicsItemFromAtom(added);
}

void ViewMapHint::_basicAlterationDone(const QList<RPZAtom::Id> &updatedIds, const Payload::Alteration &type) {
    OrderedGraphicsItems toUpdate;

    for (const auto id : updatedIds) {
        QGraphicsItem* item = nullptr;

        // if removed...
        if (type == Payload::Alteration::Removed) {
            item = this->_GItemsById.take(id);
            this->_ownedTokenIds.remove(id);
        } else {
            item = this->_GItemsById.value(id);
        }

        toUpdate.insert(id, item);
    }

    emit requestingUIAlteration(type, toUpdate);
}

void ViewMapHint::_updatesDone(const QList<RPZAtom::Id> &updatedIds, const RPZAtom::Updates &updates) {
    QList<QGraphicsItem*> toUpdate;

    for (const auto id : updatedIds) {
        toUpdate += this->_GItemsById.value(id);
    }

    emit requestingUIUpdate(toUpdate, updates);
}

void ViewMapHint::_updatesDone(const RPZAtom::ManyUpdates &updates) {
    QHash<QGraphicsItem*, RPZAtom::Updates> toUpdate;

    for (auto i = updates.constBegin(); i != updates.constEnd(); i++) {
        auto gi = this->_GItemsById.value(i.key());
        toUpdate.insert(gi, i.value());
    }

    emit requestingUIUpdate(toUpdate);
}

/////////////////////////////////
// END AtomsStorage Overriding //
/////////////////////////////////
