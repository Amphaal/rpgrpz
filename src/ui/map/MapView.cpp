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

#include "MapView.h"

MapView::MapView(QWidget *parent) : QGraphicsView(parent), MV_Manipulation(this), MV_HUDLayout(this) {
    this->setScene(new QGraphicsScene);

    this->_walkingCursor = QCursor(QStringLiteral(u":/icons/app/tools/walking.png"));
    this->_pingCursor = QCursor(QStringLiteral(u":/icons/app/tools/ping.png"), 7, 15);
    this->_quickDrawCursor = QCursor(QStringLiteral(u":/icons/app/tools/pencil.png"), 15, 15);
    this->_measureCursor = QCursor(QStringLiteral(u":/icons/app/tools/measuring.png"), 15, 15);

    // init
    this->_menuHandler = new AtomsContextualMenuHandler(this);
    this->_atomActionsHandler = new AtomActionsHandler(this, this);
    this->_atomDrawingAssist = new AtomDrawingAssist(this);
    this->_quickDrawingAssist = new QuickDrawingAssist(this);
    this->_pingAssist = new PingAssist(this);

    // OpenGL backend activation
    QGLFormat format;
    format.setSampleBuffers(true);
    format.setDirectRendering(true);
    format.setAlpha(true);

    // define viewport
    auto vp = new QGLWidget(format);
    this->setViewport(vp);
    AppContext::defineMapWidget(vp);

    // hide scrollbars
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->setRubberBandSelectionMode(Qt::ItemSelectionMode::IntersectsItemBoundingRect);  // rubberband UC optimization

    this->setMouseTracking(true);  // activate mouse tracking for ghost

    this->resetTool();

    this->_handleHintsSignalsAndSlots();

    // force scaling to enable drag mode (bug ?)
    this->scale(.99, .99);
}

void MapView::_handleHintsSignalsAndSlots() {
    // on map loading, set/unset placeholder...
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
        [=]() {
            Clipboard::clear();
            this->displayHeavyLoadPlaceholder();
    });
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessed,
        [=]() {
            this->endHeavyLoadPlaceholder();
            this->goToDefaultViewState();
    });

    QObject::connect(
        HintThread::hint(), &ViewMapHint::fogModeChanged,
        this, &MapView::_onFogModeChanged
    );

    QObject::connect(
        HintThread::hint(), &ViewMapHint::fogChanged,
        this, &MapView::_onFogChanged
    );

    QObject::connect(
        HintThread::hint(), QOverload<const Payload::Alteration &, const OrderedGraphicsItems &>::of(&ViewMapHint::requestingUIAlteration),
        this, QOverload<const Payload::Alteration &, const OrderedGraphicsItems &>::of(&MapView::_onUIAlterationRequest)
    );
    QObject::connect(
        HintThread::hint(), QOverload<const Payload::Alteration &, const QList<QGraphicsItem*>&>::of(&ViewMapHint::requestingUIAlteration),
        this, QOverload<const Payload::Alteration &, const QList<QGraphicsItem*>&>::of(&MapView::_onUIAlterationRequest)
    );

    QObject::connect(
        HintThread::hint(), QOverload<const QHash<QGraphicsItem*, RPZAtom::Updates>&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QHash<QGraphicsItem*, RPZAtom::Updates>&>::of(&MapView::_onUIUpdateRequest)
    );

    QObject::connect(
        HintThread::hint(), QOverload<const QList<QGraphicsItem*>&, const RPZAtom::Updates&, bool>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QList<QGraphicsItem*>&, const RPZAtom::Updates&, bool>::of(&MapView::_onUIUpdateRequest)
    );

    QObject::connect(
        HintThread::hint(), &ViewMapHint::changedOwnership,
        this, &MapView::_onOwnershipChanged
    );

    QObject::connect(
        HintThread::hint(), &AtomsStorage::mapSetup,
        [=](const RPZMapParameters &mParams,  const RPZFogParams &fParams) {
            this->_currentMapParameters = mParams;
    });

    // call debouncer on selection
    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        [=]() {
            if (this->_ignoreSelectionChangedEvents) return;
            this->_notifySelection();
    });

    QObject::connect(
        MapTools::get(), &MapTools::toolRequested,
        this, &MapView::_onToolRequested
    );
}

void MapView::_onToolRequested(const MapTool &tool, bool enabled) {
    this->_changeTool(enabled ? tool : MapTool::Default);
}

void MapView::_onFogModeChanged(const RPZFogParams::Mode &newMode) {
    auto request = HintThread::hint()->fogItem()->setFogMode(newMode);
    this->_mayFogUpdateAtoms(request);
}

void MapView::_onFogChanged(const QList<QPolygonF> &updatedFog) {
    auto request = HintThread::hint()->fogItem()->updateFog(updatedFog);
    this->_mayFogUpdateAtoms(request);
}

//
//
//

void MapView::_notifySelection() {
    auto selected = this->scene()->selectedItems();

    // notify
    HintThread::hint()->notifySelectedItems(selected);
}

void MapView::_metadataUpdatePostProcess(const QList<QGraphicsItem*> &FoWSensitiveItems) {
    // may trigger MapViewAnimator::triggerQueuedAnimations(), prevent calling it twice in a row
    auto mustTriggerAnimations = true;
    if (FoWSensitiveItems.count()) {
        auto request = HintThread::hint()->fogItem()->visibilityChangeFromList(FoWSensitiveItems);
        this->_mayFogUpdateAtoms(request);
        if (!request.nowInvisible.count() && !request.nowVisible.count()) mustTriggerAnimations = false;
    }

    if (mustTriggerAnimations) MapViewAnimator::triggerQueuedAnimations();
}

void MapView::_onUIUpdateRequest(const QHash<QGraphicsItem*, RPZAtom::Updates> &toUpdate) {
    QList<QGraphicsItem*> needFoWCheck;

    for (auto i = toUpdate.constBegin(); i != toUpdate.constEnd(); i++) {
        auto gi = i.key();
        auto updates = i.value();

        AtomConverter::updateGraphicsItemFromMetadata(gi, updates);

        // check FoW changes
        if (RPZAtom::mustTriggerFoWCheck(updates.keys())) needFoWCheck += gi;
    }

    this->_metadataUpdatePostProcess(needFoWCheck);
}

void MapView::_onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates, bool isPreview) {
    for (const auto item : toUpdate) {
        AtomConverter::updateGraphicsItemFromMetadata(item, updates);
    }

    auto mustTriggerFoWCheck = isPreview ? false : RPZAtom::mustTriggerFoWCheck(updates.keys());
    this->_metadataUpdatePostProcess(mustTriggerFoWCheck ? toUpdate : QList<QGraphicsItem *>());
}

void MapView::_onOwnershipChanged(const QList<QGraphicsItem*> changing, bool owned) {
    // standard handling
    if (owned || !this->_walkingHelper) {
        for (auto item : changing) {
            if (auto casted = dynamic_cast<MapViewToken*>(item)) casted->setOwned(owned);
        }
    } else {
        // specific handling when walking helper is being used and losing ownership
        auto foundTBW = false;

        for (auto item : changing) {
            // if "to be walked" has changed ownership, tag for tool change
            if (!foundTBW && WalkingHelper::isToBeWalked(this->_walkingHelper, item)) foundTBW = true;

            // change ownership
            if (auto casted = dynamic_cast<MapViewToken*>(item)) {
                casted->setOwned(owned);
            }
        }

        // if currently walked token has ownership revoked, cancel walking
        if (foundTBW) this->_changeTool(MapTool::Default);
    }
}

void MapView::_addItemToScene(QGraphicsItem* item) {
    this->scene()->addItem(item);
    if (auto animated = dynamic_cast<RPZAnimated*>(item)) {
        animated->triggerAnimation();
    }
}

void MapView::_onUIAlterationRequest(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter) {
    OrderedGraphicsItems re;
    auto i = 0;
    for (const auto item : toAlter) {
        re.insert(i, item);
        i++;
    }
    this->_onUIAlterationRequest(type, re);
}

bool MapView::_mayFogUpdateAtoms(const MapViewFog::FogChangingVisibility &itemsWhoChanged) const {
    // visible
    RPZAtom::Updates visible { { RPZAtom::Parameter::CoveredByFog, false } };
    for (auto item : itemsWhoChanged.nowVisible) {
        AtomConverter::updateGraphicsItemFromMetadata(
            item,
            visible
        );
    }

    // invisible
    RPZAtom::Updates invisible { { RPZAtom::Parameter::CoveredByFog, true } };
    for (auto item : itemsWhoChanged.nowInvisible) {
        AtomConverter::updateGraphicsItemFromMetadata(
            item,
            invisible
        );
    }

    auto mustTriggerAnimations = itemsWhoChanged.nowInvisible.count() || itemsWhoChanged.nowVisible.count();
    if (mustTriggerAnimations) MapViewAnimator::triggerQueuedAnimations();

    return mustTriggerAnimations;
}

void MapView::_onUIAlterationRequest(const Payload::Alteration &type, const OrderedGraphicsItems &toAlter) {
    // prevent circual selection
    QSignalBlocker b(this->scene());

    if (type == Payload::Alteration::Selected) this->scene()->clearSelection();
    if (type == Payload::Alteration::Reset) {
        // reset tool
        this->resetTool();

        // before clearing whole scene
        this->_atomDrawingAssist->clearDrawing();
        this->_quickDrawingAssist->clearDrawings();
        MapViewAnimator::clearAnimations();

        // clear and change rect
        this->scene()->clear();
        this->scene()->setSceneRect(this->_currentMapParameters.sceneRect());

        // add fog first
        this->_addItemToScene(HintThread::hint()->fogItem());

        // reset view
        this->goToDefaultViewState();

        // setup loader
        this->setupHeavyLoadPlaceholder(toAlter.count());
    }

    auto currentTool = this->_getCurrentTool();

    for (auto i = toAlter.begin(); i != toAlter.end(); i++) {
        auto &item = i.value();

        switch (type) {
            case Payload::Alteration::Reset: {
                this->_addItemToScene(item);
                this->incrementHeavyLoadPlaceholder();
            }
            break;

            case Payload::Alteration::Replaced:
            case Payload::Alteration::Added: {
                this->_addItemToScene(item);

                // replace if must
                if (type == Payload::Alteration::Replaced) {
                    auto toReplace = RPZQVariant::graphicsItemToReplace(item);
                    item->stackBefore(toReplace);
                    delete toReplace;
                }

                // auto remove temporary drawing
                auto isCommitedDrawing = this->_atomDrawingAssist->compareItemToCommitedDrawing(item);
                if (isCommitedDrawing) break;

                // if not from temporary drawing, animate path
                if (auto canBeAnimated = dynamic_cast<MapViewDrawing*>(item)) {
                    MapViewAnimator::animatePath(canBeAnimated, canBeAnimated->path());
                }
            }
            break;

            case Payload::Alteration::Focused: {
                this->focusItem(item);
            }
            break;

            case Payload::Alteration::Selected: {
                item->setSelected(true);
            }
            break;

            case Payload::Alteration::Removed: {
                // if walked item is about to be deleted, change tool to default
                auto deletedItemIsToBeWalked = WalkingHelper::isToBeWalked(this->_walkingHelper, item);
                if (deletedItemIsToBeWalked && currentTool == MapTool::Walking) {
                    this->_changeTool(MapTool::Default);
                }

                delete item;
            }
            break;

            case Payload::Alteration::ToySelected: {
                auto newTool = item ? MapTool::Atom : MapTool::Default;
                auto selectedAtom = item ? HintThread::hint()->templateAtom() : RPZAtom();
                if (item) this->scene()->addItem(item);

                // change tool, proceed to unselect items
                this->_changeTool(newTool);

                // emit that template selection has changed
                AtomTemplateSelectedPayload payload(selectedAtom);
                AlterationHandler::get()->queueAlteration(HintThread::hint(), payload);
            }
            break;

            default:
            break;
        }
    }

    // prevent animations caching while map is being reset
    if (Payload::foWPreventAnimation.contains(type)) {
        MapViewAnimator::setAnimationsAllowed(false);
    }

    if (type == Payload::Alteration::Reset) {
        auto coveredItems = HintThread::hint()->fogItem()->coveredAtomItems();
        this->_mayFogUpdateAtoms(coveredItems);
        ProgressTracker::get()->heavyAlterationEnded();

    } else if (Payload::triggersFoWCheck.contains(type)) {  // check specific items for fog updates
        auto changedItemsList = toAlter.values();
        auto updates = HintThread::hint()->fogItem()->visibilityChangeFromList(changedItemsList);
        this->_mayFogUpdateAtoms(updates);

    } else if (type == Payload::Alteration::Selected) {
        auto isWalkable = _tryToInvokeWalkableHelper(toAlter.values());

        // if cant walk it, and is using walking tool, go back to default tool
        if (!isWalkable && currentTool == MapTool::Walking) {
            this->_changeTool(MapTool::Default);
        }
    }

    MapViewAnimator::setAnimationsAllowed(true);
    MapViewAnimator::triggerQueuedAnimations();
}

bool MapView::_tryToInvokeWalkableHelper(const QList<QGraphicsItem*> &toBeWalked) {
    if (!toBeWalked.count()) return false;

    // prevent if not all walkables
    for (auto item : toBeWalked) {
        if (!RPZQVariant::contextuallyOwned(item)) return false;
    }

    // clear previous walker
    this->_clearWalkingHelper();

    // create walking helper
    this->_walkingHelper = new WalkingHelper(
        this->_currentMapParameters,
        toBeWalked,
        this
    );

    return true;
}

//
//
//


//////////////////
// UI rendering //
//////////////////

void MapView::onViewRectChange() {
    emit cameraMoved();
}

void MapView::drawForeground(QPainter *painter, const QRectF &rect) {
    this->mayUpdateHeavyLoadPlaceholder(painter);
    this->mayUpdateHUD(painter, rect, this->_currentMapParameters);
}

void MapView::drawBackground(QPainter *painter, const QRectF &rect) {
    this->drawBackgroundCheckerboard(painter, rect);
}

//////////////////////
// End UI rendering //
//////////////////////

////////////
/* EVENTS */
////////////

void MapView::contextMenuEvent(QContextMenuEvent *event) {
    // prevent contextual menu if not using default tool
    if (this->_getCurrentTool() != MapTool::Default) return;

    // create menu
    this->_menuHandler->invokeMenu(this->selectedIds(), event->globalPos());
}

void MapView::mouseDoubleClickEvent(QMouseEvent *event) {
    // prevent if not using default tool
    if (this->_getCurrentTool() != MapTool::Default) return;

    // find first selectable item
    QGraphicsItem* focusable = nullptr;
    auto items = this->items(event->pos());
    for (const auto item : items) {
        if (item->flags().testFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable)) {
            focusable = item;
            break;
        }
    }
    if (!focusable) return;

    // check item is not temporary !
    if (RPZQVariant::isTemporary(focusable)) return;

    // notify focus
    HintThread::hint()->notifyFocusedItem(focusable);

    // request focus on sheet
    if (auto characterId = RPZQVariant::boundCharacterId(focusable)) {
        emit requestingFocusOnCharacter(characterId);
    }
}

void MapView::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key::Key_Up:
        case Qt::Key::Key_Down:
        case Qt::Key::Key_Left:
        case Qt::Key::Key_Right:
            event->ignore();
            return this->removeAnimatedMove(event);
            break;
    }

    QGraphicsView::keyReleaseEvent(event);
}

const QList<RPZAtom::Id> MapView::selectedIds() const {
    return HintThread::hint()->getAtomIdsFromGraphicsItems(
        this->scene()->selectedItems()
    );
}

void MapView::keyPressEvent(QKeyEvent * event) {
    switch (event->key()) {
        // ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->resetTool();
            break;

        case Qt::Key::Key_Up:
        case Qt::Key::Key_Down:
        case Qt::Key::Key_Left:
        case Qt::Key::Key_Right:
            event->ignore();
            return this->addAnimatedMove(event);
            break;
    }

    QGraphicsView::keyPressEvent(event);
}


void MapView::enterEvent(QEvent *event) {
    this->_isCursorIn = true;

    if (auto ghost = this->_displayableGhostItem()) {
        this->_mightCenterGhostWithCursor();
        ghost->setVisible(true);
    }
}

void MapView::leaveEvent(QEvent *event) {
    // special case for keeping visible when right clicking
    auto cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
    if (this->geometry().contains(cursorPosInWidget)) return;

    this->_isCursorIn = false;

    if (auto ghost = this->_displayableGhostItem()) {
        ghost->setVisible(false);
    }
}

QGraphicsItem* MapView::_displayableGhostItem() {
    if (this->_getCurrentTool() != MapTool::Atom) return nullptr;
    return HintThread::hint()->ghostItem();
}

////////////////
/* END EVENTS */
////////////////

//////////////////
/* MOUSE EVENTS */
//////////////////

// mouse click
void MapView::mousePressEvent(QMouseEvent *event) {
    // register last position
    this->_isMousePressed = true;
    auto btnPressed = event->button();

    if (btnPressed == Qt::MouseButton::MiddleButton) {
        auto tool = this->_quickTool == MapTool::Default ? MapTool::Scroll : MapTool::Default;
        this->_changeTool(tool, true);
        return;
    }

    auto currentTool = this->_getCurrentTool();

    // for atoms
    if (currentTool == MapTool::Atom) {
        // conditionnal drawing
        auto templateAtom = HintThread::hint()->templateAtom();
        auto templateType = templateAtom.type();

        // FoW...
        if (templateType == RPZAtom::Type::FogOfWar) {
            auto fogItem = HintThread::hint()->fogItem();
            fogItem->initDrawing(btnPressed == Qt::MouseButton::LeftButton ? FogChangedPayload::ChangeType::Added : FogChangedPayload::ChangeType::Removed);

            auto scenePos = this->mapToScene(event->pos());
            fogItem->drawToPoint(scenePos);

        } else if (btnPressed == Qt::MouseButton::LeftButton) {  // any other atoms
            switch (templateAtom.type()) {
                case RPZAtom::Type::Drawing:
                case RPZAtom::Type::Brush:
                    this->_atomDrawingAssist->addDrawingPoint(event->pos(), templateAtom);
                break;

                default: {
                    auto ghost = HintThread::hint()->ghostItem();
                    if (WalkingHelper::isMoveOrInsertPreventedAtPosition(this->_currentMapParameters, ghost)) break;

                    HintThread::hint()->integrateGraphicsItemAsPayload(ghost);
                }
                break;
            }
        }

    } else if (btnPressed == Qt::MouseButton::LeftButton) {  // default
        switch (currentTool) {
            case MapTool::Default: {
                this->_ignoreSelectionChangedEvents = !this->_isAnySelectableItemsUnderCursor(event->pos());

                if (!this->_ignoreSelectionChangedEvents && this->_walkingHelper) {
                    this->_changeTool(MapTool::Walking);
                } else {
                    QGraphicsView::mousePressEvent(event);  // allows rubber band selection
                }
            }
            break;

            case MapTool::Measure : {
                this->_clearMeasurementHelper();
                this->_measurementHelper = new MeasurementHelper(this->_currentMapParameters, event->pos(), this);
                this->scene()->addItem(this->_measurementHelper);
            }
            break;

            case MapTool::Ping : {
                this->_pingAssist->generatePing(event->pos());
            }
            break;

            case MapTool::QuickDraw: {
                this->_quickDrawingAssist->addDrawingPoint(event->pos());
            }
            break;

            case MapTool::Scroll: {
                QGraphicsView::mousePressEvent(event);  // allows move with mouse
            }
            break;

            default: {}
            break;
        }
    }
}

// on movement
void MapView::mouseMoveEvent(QMouseEvent *event) {
    this->_mightCenterGhostWithCursor();

    auto currentTool = this->_getCurrentTool();

    if (currentTool == MapTool::Atom) {
        auto type = HintThread::hint()->templateAtom().type();

        switch (type) {
            case RPZAtom::Type::FogOfWar: {
                if (!this->_isMousePressed) break;

                auto scenePos = this->mapToScene(event->pos());
                HintThread::hint()->fogItem()->drawToPoint(scenePos);
            }
            break;

            case RPZAtom::Type::Drawing:
            case RPZAtom::Type::Brush:
                this->_atomDrawingAssist->updateDrawingPath(event->pos());
            break;

            default:
                break;
        }

    } else if (currentTool == MapTool::Walking) {
        this->_mightUpdateWalkingHelperPos();
    } else if (currentTool == MapTool::QuickDraw) {
        this->_quickDrawingAssist->updateDrawingPath(event->pos());
    } else if (currentTool == MapTool::Scroll && this->_isMousePressed) {
        emit cameraMoved();
    }

    QGraphicsView::mouseMoveEvent(event);
}

void MapView::_mightUpdateWalkingHelperPos() {
    if (this->_walkingHelper) {
        this->_walkingHelper->update();
    }
}

// mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {
    this->_isMousePressed = false;
    auto btnPressed = event->button();
    auto currentTool = this->_getCurrentTool();

    // as atom
    if (currentTool == MapTool::Atom) {
        // if fog of war
        if (HintThread::hint()->templateAtom().type() == RPZAtom::Type::FogOfWar) {
            auto drawn = HintThread::hint()->fogItem()->commitDrawing();

            // if any drawn
            if (drawn.count()) {
                auto changeType = btnPressed == Qt::MouseButton::LeftButton ? FogChangedPayload::ChangeType::Added : FogChangedPayload::ChangeType::Removed;
                FogChangedPayload payload(changeType, drawn);
                AlterationHandler::get()->queueAlteration(HintThread::hint(), payload);
            }
        }
    }

    // if left clicking
    if (btnPressed == Qt::MouseButton::LeftButton) {
        // commit sticky drawing
        this->_atomDrawingAssist->onMouseRelease();
        this->_quickDrawingAssist->onMouseRelease();

        switch (currentTool) {
            case MapTool::Measure: {
                this->_clearMeasurementHelper();
            }
            break;

            case MapTool::Default: {
                // if something moved ?
                HintThread::hint()->mightNotifyMovement(this->scene()->selectedItems());

                // trigger items selection
                QGraphicsView::mouseReleaseEvent(event);

                // update selection
                if (this->_ignoreSelectionChangedEvents) this->_notifySelection();
                this->_ignoreSelectionChangedEvents = false;
            }
            break;

            case MapTool::Walking: {
                HintThread::hint()->notifyWalk(this->_walkingHelper->destinations());
                this->resetTool();
            }
            break;

            case MapTool::Scroll: {
                // trigger hand release
                QGraphicsView::mouseReleaseEvent(event);
            }
            break;

            default: {}
            break;
        }
    }
}

void MapView::_clearMeasurementHelper() {
    if (this->_measurementHelper) delete this->_measurementHelper;
    this->_measurementHelper = nullptr;
}

bool MapView::_isAnySelectableItemsUnderCursor(const QPoint &cursorPosInWindow) const {
    auto cursorScenePos = this->mapToScene(cursorPosInWindow);

    for (const auto colliding : this->scene()->items(cursorScenePos)) {
        if (colliding->flags().testFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable))
            return true;
    }

    return false;
}

void MapView::wheelEvent(QWheelEvent *event) {
    // make sure no button is pressed
    if (this->_isMousePressed) return;

    this->animateScroll(event, this->_currentMapParameters);
}

void MapView::scrollFromMinimap(QWheelEvent *event) {
    this->animateScroll(event, this->_currentMapParameters);
}

void MapView::focusFromMinimap(const QPointF &scenePoint) {
    this->centerOn(scenePoint);
    this->onViewRectChange();
}

//////////////////////
/* END MOUSE EVENTS */
//////////////////////

//////////
/* TOOL */
//////////

// returns tool
MapTool MapView::_getCurrentTool() const {
    return this->_quickTool == MapTool::Default ? this->_tool : this->_quickTool;
}

void MapView::resetTool() {
    this->_quickTool = MapTool::Default;
    this->_changeTool(MapTool::Default);
}

void MapView::_changeTool(MapTool newTool, const bool quickChange) {
    // end drawing if any
    this->_atomDrawingAssist->mayCommitDrawing();
    this->_quickDrawingAssist->onMouseRelease();
    this->_clearMeasurementHelper();

    // prevent the usage of Atom tool if not host able
    if (!Authorisations::isHostAble() && newTool == MapTool::Atom) return;

    // if quick change asked
    if (quickChange) {
        this->_quickTool = newTool;

        // if unselecting quicktool
        if (newTool == MapTool::Default) newTool = this->_tool;

    } else {
        // if standard tool change
        // since clearSelection wont trigger notification, hard call notification on reset
        if (this->_tool == MapTool::Atom && newTool == MapTool::Default) {
            this->_notifySelection();
        }

        this->_tool = newTool;

        // notify MapTools
        MapTools::get()->onToolChange(this->_tool);

        if (newTool != MapTool::Walking) {
            this->scene()->clearSelection();
        }
    }

    // if a quicktool is selected
    if (this->_quickTool != MapTool::Default) {
        newTool = this->_quickTool;
    }

    // destroy walking helper
    if (this->_tool != MapTool::Walking && this->_walkingHelper) {
        this->_clearWalkingHelper();
    }

    // define interactivity
    this->setInteractive(newTool == MapTool::Default ? true : false);

    // depending on tool
    switch (newTool) {
        case MapTool::Atom: {
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            switch (HintThread::hint()->templateAtom().type()) {
                case RPZAtom::Type::Drawing:
                    this->setCursor(Qt::CrossCursor);
                    break;
                case RPZAtom::Type::Object:
                    this->setCursor(Qt::ClosedHandCursor);
                    break;
                case RPZAtom::Type::Text:
                    this->setCursor(Qt::IBeamCursor);
                    break;
                default:
                    this->setCursor(Qt::CrossCursor);
                    break;
            }

            if (auto ghost = HintThread::hint()->ghostItem()) {
                ghost->setVisible(this->_isCursorIn);
            }
        }
        break;

        case MapTool::Scroll: {
            this->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

            if (auto ghost = HintThread::hint()->ghostItem()) {
                ghost->setVisible(false);  // force hidden if any
            }
        }
        break;

        case MapTool::Walking: {
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(this->_walkingCursor);

            // add it to scene
            this->scene()->addItem(this->_walkingHelper);
        }
        break;

        case MapTool::Ping: {
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(this->_pingCursor);
        }
        break;

        case MapTool::QuickDraw: {
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(this->_quickDrawCursor);
        }
        break;

        case MapTool::Measure: {
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(this->_measureCursor);
        }
        break;

        case MapTool::Default:
        default: {
            this->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
            this->setCursor(Qt::ArrowCursor);
        }
        break;
    }

    // may update ghost pos
    this->_mightCenterGhostWithCursor();
}

void MapView::_clearWalkingHelper() {
    delete this->_walkingHelper;
    this->_walkingHelper = nullptr;
}

// on received action
void MapView::onActionRequested(const MapAction &action) {
    switch (action) {
        case MapAction::ResetView:
            this->goToDefaultViewState();
            break;

        case MapAction::ResetTool:
            this->resetTool();
            break;

        default:
            break;
    }
}

void MapView::onHelperActionTriggered(QAction *action) {
    this->setForegroundBrush(Qt::NoBrush);   // force foreground re-drawing
}


void MapView::_mightCenterGhostWithCursor() {
    // update ghost item position relative to cursor
    if (auto ghost = this->_displayableGhostItem()) {
        // map cursor pos to widget
        auto cursorPos = this->mapFromGlobal(QCursor::pos());
        auto ghostCenter = ghost->boundingRect().center();
        QPointF cursorPosInScene;

        // check if ignores transformation
        if (ghost->flags().testFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations)) {
            cursorPos -= ghostCenter.toPoint();
            cursorPosInScene = this->mapToScene(cursorPos);
        } else {
            cursorPosInScene = this->mapToScene(cursorPos);
            cursorPosInScene -= ghostCenter;
        }

        // if grid movement and alignable, stick to grid
        if (auto gridBound = dynamic_cast<const RPZGridBound*>(ghost)) {
            gridBound->adaptativePointAlignementToGrid(this->_currentMapParameters, cursorPosInScene);
        }

        ghost->setPos(cursorPosInScene);
    }
}

//////////////
/* END TOOL */
//////////////

void MapView::onAnimationManipulationTickDone() {
    this->_mightCenterGhostWithCursor();
    this->_mightUpdateWalkingHelperPos();
}
