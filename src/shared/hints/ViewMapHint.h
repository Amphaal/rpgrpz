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

#pragma once

#include <QMultiHash>
#include <QGraphicsView>
#include <QWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QPixmap>

#include "src/shared/models/toy/RPZAsset.hpp"

#include "src/shared/renderer/AtomRenderer.h"

#include "src/shared/payloads/Payloads.h"
#include "src/ui/toysBox/_base/ToysTreeViewModel.h"

#include "AtomsStorage.h"

#include "src/helpers/RPZQVariant.hpp"

using OrderedGraphicsItems = QMap<RPZAtom::Id, QGraphicsItem*>;

class ViewMapHint : public AtomsStorage {
    Q_OBJECT

 public:
    struct SingleSelectionInteractible {
        bool isInteractive = false;
        RPZAtom interactible;
    };

    ViewMapHint();

    // might be called by another thread, safe
    RPZAtom::Id integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem) const;  // safe
    void notifyWalk(const QHash<QGraphicsItem*, QPointF> &toWalk);  // safe
    const RPZAtom templateAtom() const;  // safe
    QGraphicsItem* ghostItem() const;  // safe
    MapViewFog* fogItem() const;  // safe

    const QList<RPZAtom::Id> getAtomIdsFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const;  // safe
    RPZAtom::Id getAtomIdFromGraphicsItem(const QGraphicsItem* toFetch) const;  // safe

    QGraphicsItem* generateGraphicsFromTemplate(bool hiddenAsDefault = false) const;  // safe

    void mightNotifyMovement(const QList<QGraphicsItem*> &itemsWhoMightHaveMoved);  // safe
    void notifySelectedItems(const QList<QGraphicsItem*> &selectedItems);  // safe
    void notifyFocusedItem(QGraphicsItem* focusedItem);  // safe
    void setDefaultLayer(int layer);  // safe
    void setDefaultVisibility(int checkboxState);  // safe
    void mightUpdateOwnedTokens(const RPZUser &owner);  // safe
    void defineImpersonatingCharacter(const RPZCharacter::Id &toImpersonate = 0);  // safe

    // handle preview alteration before real payload
    void handlePreviewRequest(const AtomsSelectionDescriptor &selectionDescriptor, const RPZAtom::Parameter &parameter, const QVariant &value);

 signals:
    void requestingUIAlteration(const Payload::Alteration &type, const OrderedGraphicsItems &toAlter);
    void requestingUIAlteration(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter);
    void requestingUIUpdate(const QHash<QGraphicsItem*, RPZAtom::Updates> &toUpdate);
    void requestingUIUpdate(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates, bool isPreview = false);
    void atomDescriptorUpdated(const RPZAtom &base = RPZAtom());
    void changedOwnership(QList<QGraphicsItem*> changing, bool owned);
    void fogModeChanged(const RPZFogParams::Mode &newMode);
    void fogChanged(const QList<QPolygonF> &updatedFog);

 protected:
    void _handleAlterationRequest(const AlterationPayload &payload) override;

 private:
    // alter template Atom
    mutable QMutex _m_ghostItem;
    QGraphicsItem* _ghostItem = nullptr;

    mutable QMutex _m_templateToy;
    RPZToy _templateToy;

    mutable QMutex _m_templateAtom;
    RPZAtom _templateAtom;

    mutable QMutex _m_GItemsById;
    QMap<RPZAtom::Id, QGraphicsItem*> _GItemsById;

    mutable QMutex _m_fogItem;
    MapViewFog* _fogItem = nullptr;

    // atom descriptor
    ViewMapHint::SingleSelectionInteractible _singleSelectionInteractible;
    const ViewMapHint::SingleSelectionInteractible _generateSSI(const SelectedPayload* payload) const;

    QMultiHash<RPZAsset::Hash, QGraphicsItem*> _missingAssetHashesFromDb;

    // owning
    QSet<RPZAtom::Id> _ownedTokenIds;
    RPZCharacter::Id _myCharacterId = 0;
    bool _hasOwnershipOf(const RPZAtom &atom) const;
    const QList<QGraphicsItem*> _gis(const QList<RPZAtom::Id> &atomIds) const;  // not safe !
    void _checkForOwnedTokens();

    // helpers
    QGraphicsItem* _generateGhostItem(const RPZToy &toy, QGraphicsItem* &oldGhostToDelete);
    QGraphicsItem* _buildGraphicsItemFromAtom(const RPZAtom &atomToBuildFrom);
    void _crossBindingAtomWithGI(const RPZAtom &atom, QGraphicsItem* gi);
    void _updateTemplateAtom(RPZAtom::Updates updates);
    void _resetTemplate(const RPZToy &from);
    void _mightUpdateAtomDescriptor(const QList<RPZAtom::Id> &idsUpdated);

    // missing assets tracking
    void _replaceMissingAssetPlaceholders(const RPZAsset &metadata);  // safe

    // augmenting AtomsStorage
    void _atomAdded(const RPZAtom &added) override;

    void _basicAlterationDone(const QList<RPZAtom::Id> &updatedIds, const Payload::Alteration &type) override;
    void _updatesDone(const QList<RPZAtom::Id> &updatedIds, const RPZAtom::Updates &updates) override;
    void _updatesDone(const RPZAtom::ManyUpdates &updates) override;
    void _atomOwnerChanged(const RPZAtom::Id &target, const RPZCharacter::Id &newOwner) override;
    void _fogUpdated(const QList<QPolygonF> &updatedFog) override;
};
