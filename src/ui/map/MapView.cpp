#include "MapView.h"

MapView::MapView(QWidget *parent) : QGraphicsView(parent), MV_Manipulation(this), MV_HUDLayout(this) {

    this->setScene(new QGraphicsScene); //dummy scene

    this->_walkingCursor = QCursor(QStringLiteral(u":/icons/app/tools/walking.png"));

    //init
    this->_menuHandler = new AtomsContextualMenuHandler(this);
    this->_atomActionsHandler = new AtomActionsHandler(this, this);
    this->_drawingAssist = new DrawingAssist(this);

    //OpenGL backend activation
    QGLFormat format;
    format.setSampleBuffers(true);
    format.setDirectRendering(true);
    format.setAlpha(true);

    //define viewport
    auto vp = new QGLWidget(format);
    this->setViewport(vp);
    AppContext::defineMapWidget(vp);

    //hide scrollbars
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    this->setRubberBandSelectionMode(Qt::ItemSelectionMode::IntersectsItemBoundingRect); //rubberband UC optimization

    this->setMouseTracking(true); //activate mouse tracking for ghost 

    this->_resetTool();

    this->_handleHintsSignalsAndSlots();

    //force scaling to enable drag mode (bug ?)
    this->scale(.99, .99);

}

void MapView::_handleHintsSignalsAndSlots() {

    //on map loading, set/unset placeholder...
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
        [=]() {
            Clipboard::clear();
            this->displayHeavyLoadPlaceholder();
        }
    );
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessed,
        [=]() {
            this->endHeavyLoadPlaceholder();
            this->goToDefaultViewState();
            this->_mightUpdateTokens();
        }
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
        HintThread::hint(), QOverload<const QList<QGraphicsItem*>&, const RPZAtom::Updates&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QList<QGraphicsItem*>&, const RPZAtom::Updates&>::of(&MapView::_onUIUpdateRequest)
    );

    QObject::connect(
        HintThread::hint(), &ViewMapHint::changedOwnership,
        this, &MapView::_onOwnershipChanged
    );

    QObject::connect(
        HintThread::hint(), &AtomsStorage::mapParametersChanged,
        [=](const RPZMapParameters &mParams) {
            this->_currentMapParameters = mParams;
        }
    );

    //call debouncer on selection
    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        [=]() {
            if(this->_ignoreSelectionChangedEvents) return;
            this->_notifySelection();
        }
    );


}

void MapView::_mightUpdateTokens() {
    
    if(!Authorisations::isHostAble()) return;
    if(!this->_rpzClient) return;
        
    //update token values from owners
    for(const auto &user : this->_rpzClient->sessionUsers()) {
        HintThread::hint()->mightUpdateOwnedTokens(user);
    }

}

//
//
//

void MapView::_notifySelection() {
    HintThread::hint()->notifySelectedItems(
        this->scene()->selectedItems()
    );
}

void MapView::_updateItemValue(QGraphicsItem* item, const RPZAtom::Updates &updates) {
    AtomConverter::updateGraphicsItemFromMetadata(
        item,
        updates
    );
}

void MapView::_onUIUpdateRequest(const QHash<QGraphicsItem*, RPZAtom::Updates> &toUpdate) {
    
    for(auto i = toUpdate.constBegin(); i != toUpdate.constEnd(); i++) {
        this->_updateItemValue(i.key(), i.value());
    }

    MapViewAnimator::triggerQueuedAnimations();

}

void MapView::_onOwnershipChanged(const QList<QGraphicsItem*> changing, bool owned) {
    
    //standard handling
    if(owned || !this->_toWalk) {
        for(auto item : changing) {
            if(auto casted = dynamic_cast<MapViewToken*>(item)) casted->setOwned(owned);
        }
    } 
    
    //specific handling when walking helper is being used and losing ownership
    else {
        
        auto foundTBW = false;

        for(auto item : changing) {
            if(!foundTBW && item == this->_toWalk) foundTBW = true;
            if(auto casted = dynamic_cast<MapViewToken*>(item)) casted->setOwned(owned);
        }
        
        //if currently walked token has ownership revoked, cancel walking
        if(foundTBW) this->_changeTool(MapTool::Default);

    }

}

void MapView::_onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates) {

    for(const auto item : toUpdate) {
        this->_updateItemValue(item, updates);
    }
    
    MapViewAnimator::triggerQueuedAnimations();

}

void MapView::_addItemToScene(QGraphicsItem* item) {

    this->scene()->addItem(item);

    if(auto token = dynamic_cast<MapViewToken*>(item)) {
        token->triggerAnimation();
    }

}

void MapView::_onUIAlterationRequest(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter) {

    OrderedGraphicsItems re;
    
    auto i = 0;
    for(const auto item : toAlter) {
        re.insert(i, item);
        i++;
    }

    this->_onUIAlterationRequest(type, re);

}

void MapView::_onUIAlterationRequest(const Payload::Alteration &type, const OrderedGraphicsItems &toAlter) {
    
    //prevent circual selection
    QSignalBlocker b(this->scene());

    if(type == Payload::Alteration::Selected) this->scene()->clearSelection();
    if(type == Payload::Alteration::Reset) {
        
        //reset tool
        this->_resetTool();

        //before clearing whole scene
        this->_drawingAssist->clearDrawing(); 
        MapViewAnimator::clearAnimations();

        //clear and change rect
        this->scene()->clear();
        this->scene()->setSceneRect(this->_currentMapParameters.sceneRect());

        //reset view
        this->goToDefaultViewState();

        //setup loader
        this->setupHeavyLoadPlaceholder(toAlter.count());

    }   

    auto currentTool = this->_getCurrentTool();

    for(auto i = toAlter.begin(); i != toAlter.end(); i++) {
        
        auto &id = i.key();
        auto &item = i.value();

        switch(type) {

            case Payload::Alteration::Reset: {
                this->_addItemToScene(item);      
                this->incrementHeavyLoadPlaceholder();
            }
            break;

            case Payload::Alteration::Added: {

                this->_addItemToScene(item);

                //auto remove temporary drawing
                auto isCommitedDrawing = this->_drawingAssist->compareItemToCommitedDrawing(item);
                if(isCommitedDrawing) break;

                //if not from temporary drawing, animate path
                if(auto canBeAnimated = dynamic_cast<MapViewDrawing*>(item)) {
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
                
                //if walked item is about to be deleted, change tool to default
                if(item == this->_toWalk && currentTool == MapTool::Walking) {
                    this->_changeTool(MapTool::Default);
                }

                delete item;

            }
            break;

            case Payload::Alteration::ToySelected: {
                auto newTool = item ? MapTool::Atom : MapTool::Default;
                auto selectedAtom = item ? HintThread::hint()->templateAtom() : RPZAtom();
                if(item) this->scene()->addItem(item);

                //change tool, proceed to unselect items
                this->_changeTool(newTool);

                //emit that template selection has changed
                AtomTemplateSelectedPayload payload(selectedAtom);
                AlterationHandler::get()->queueAlteration(HintThread::hint(), payload);
            }
            break;

            default:
            break;
        }
    }

    if(type == Payload::Alteration::Reset) {
        ProgressTracker::get()->heavyAlterationEnded();
    }

    else if(type == Payload::Alteration::Selected) {
        
        auto firstItem = toAlter.values().value(0);
        auto isWalkable = _tryToInvokeWalkableHelper(firstItem);

        //if cant walk it, and is using walking tool, go back to default tool
        if(!isWalkable && currentTool == MapTool::Walking) {
            this->_changeTool(MapTool::Default);
        }

    }

    MapViewAnimator::triggerQueuedAnimations();

}

bool MapView::_tryToInvokeWalkableHelper(QGraphicsItem * toBeWalked) {
    
    if(!toBeWalked) return false;
    if(!this->_isMousePressed) return false;

    //prevent if not walkable
    auto isWalkable = RPZQVariant::allowedToBeWalked(toBeWalked);
    if(!isWalkable) return false;
        
    //clear previous walker
    this->_clearWalkingHelper();

    //create walking helper
    this->_toWalk = toBeWalked;
    this->_walkingHelper = new MapViewWalkingHelper(
        this->_currentMapParameters, 
        this->_toWalk, 
        this
    );

    //define tool
    this->_changeTool(MapTool::Walking);

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

    //prevent contextual menu if not using default tool
    if(this->_getCurrentTool() != MapTool::Default) return;

    //create menu
    this->_menuHandler->invokeMenu(
        this->selectedIds(), 
        event->globalPos()
    );

}

void MapView::mouseDoubleClickEvent(QMouseEvent *event) {
    
    //prevent if not using default tool
    if(this->_getCurrentTool() != MapTool::Default) return;

    //find first selectable item
    QGraphicsItem* focusable = nullptr;
    auto items = this->items(event->pos());
    for(const auto item : items) {
        if(item->flags().testFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable)) {
            focusable = item;
            break;
        }
    }
    if(!focusable) return;

    //check item is not temporary !
    if(RPZQVariant::isTemporary(focusable)) return;

    //notify focus
    HintThread::hint()->notifyFocusedItem(focusable);

    //request focus on sheet
    if(auto characterId = RPZQVariant::boundCharacterId(focusable)) {
        emit requestingFocusOnCharacter(characterId);
    }

}

void MapView::keyReleaseEvent(QKeyEvent *event) {

    switch(event->key()) {
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

    switch(event->key()) {

        //ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->_resetTool();
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

    if(auto ghost = this->_displayableGhostItem()) {
        this->_mightCenterGhostWithCursor();
        ghost->setVisible(true);
    }  

}

void MapView::leaveEvent(QEvent *event) {
    
    //special case for keeping visible when right clicking
    auto cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
    if(this->geometry().contains(cursorPosInWidget)) return;

    this->_isCursorIn = false;

    if(auto ghost = this->_displayableGhostItem()) {
        ghost->setVisible(false);
    }

}

QGraphicsItem* MapView::_displayableGhostItem() {
    if(this->_getCurrentTool() != MapTool::Atom) return nullptr;
    return HintThread::hint()->ghostItem();
}

////////////////
/* END EVENTS */
////////////////

//////////////////
/* MOUSE EVENTS */
//////////////////

//mouse click
void MapView::mousePressEvent(QMouseEvent *event) {

    //register last position
    this->_isMousePressed = true;

    switch(event->button()) {

        case Qt::MouseButton::MiddleButton: {
            auto tool = this->_quickTool == MapTool::Default ? MapTool::Scroll : MapTool::Default;
            this->_changeTool(tool, true);
        }
        break;

        case Qt::MouseButton::LeftButton: {

            switch(this->_getCurrentTool()) {
                
                case MapTool::Default: {
                    this->_ignoreSelectionChangedEvents = !this->_isAnySelectableItemsUnderCursor(event->pos());
                }
                break;

                case MapTool::Atom: {

                    //conditionnal drawing
                    switch(HintThread::hint()->templateAtom().type()) {

                        case RPZAtom::Type::Drawing:
                        case RPZAtom::Type::Brush:
                            this->_drawingAssist->addDrawingPoint(event->pos());
                        break;

                        default: {

                            auto ghost = HintThread::hint()->ghostItem();
                            if(this->_preventMoveOrInsertAtPosition(ghost)) break;

                            HintThread::hint()->integrateGraphicsItemAsPayload(ghost);

                        }
                        break;

                    }

                }
                break;

                default:
                break;

            }

            //allows rubber band selection
            QGraphicsView::mousePressEvent(event);

        }
        break;

        default:
            break;
    }

}

bool MapView::_preventMoveOrInsertAtPosition(QGraphicsItem *toCheck, const QPointF &toCheckAt) {
    
    auto atPosRect = toCheck->sceneBoundingRect();
    if(!toCheckAt.isNull()) atPosRect.moveCenter(toCheckAt);

    if(this->_currentMapParameters.movementSystem() != RPZMapParameters::MovementSystem::Grid) return false;
    if(!RPZQVariant::isGridBound(toCheck)) return false;

    for(const auto colliding : this->scene()->items(atPosRect)) {
        if(colliding == toCheck) continue;
        if(RPZQVariant::isGridBound(colliding)) return true;
    }

    return false;
}

    

//on movement
void MapView::mouseMoveEvent(QMouseEvent *event) {

    this->_mightCenterGhostWithCursor();

    auto currentTool = this->_getCurrentTool();

    if(currentTool == MapTool::Atom) {
        
        auto type = HintThread::hint()->templateAtom().type();
        
        switch(type) {
            case RPZAtom::Type::Drawing:
            case RPZAtom::Type::Brush:
                this->_drawingAssist->updateDrawingPath(event->pos(), type);
            break;

            default:
                break;
        }

    }

    else if(currentTool == MapTool::Walking) {
        this->_mightUpdateWalkingHelperPos();
    }

    else if(currentTool == MapTool::Scroll && this->_isMousePressed) {
        emit cameraMoved();
    }

    QGraphicsView::mouseMoveEvent(event);
    
}

void MapView::_mightUpdateWalkingHelperPos() {
    if(this->_walkingHelper) {
        this->_walkingHelper->update();
    }
}

//mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {

    this->_isMousePressed = false;

    switch(event->button()) {
        case Qt::MouseButton::LeftButton: {
            
            //if was drawing...
            this->_drawingAssist->onMouseRelease();

            switch(this->_getCurrentTool()) {
                
                case MapTool::Default: {

                    //if something moved ?
                    HintThread::hint()->mightNotifyMovement(this->scene()->selectedItems()); 

                    //trigger items selection
                    QGraphicsView::mouseReleaseEvent(event);

                    //update selection
                    if(this->_ignoreSelectionChangedEvents) this->_notifySelection();
                    this->_ignoreSelectionChangedEvents = false;

                }
                break;

                case MapTool::Walking: {

                    auto toWalkTo = this->_walkingHelper->destScenePos();

                    if(!this->_preventMoveOrInsertAtPosition(this->_toWalk, toWalkTo)) {
                        HintThread::hint()->notifyWalk(this->_toWalk, toWalkTo);
                    }

                    this->_resetTool();

                }
                break;

                case MapTool::Scroll: {

                    //trigger hand release
                    QGraphicsView::mouseReleaseEvent(event);

                };

                default:
                break;

            }
            
        }
        break;

        default:
            break;
    }

}

bool MapView::_isAnySelectableItemsUnderCursor(const QPoint &cursorPosInWindow) const {
    
    auto cursorScenePos = this->mapToScene(cursorPosInWindow);
    
    for(const auto colliding : this->scene()->items(cursorScenePos)) {
        if(colliding->flags().testFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable)) return true;
    }

    return false;

}

void MapView::wheelEvent(QWheelEvent *event) {

    //make sure no button is pressed
    if(this->_isMousePressed) return;
    
    this->animateScroll(event, this->_currentMapParameters);

};

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

/////////////
/* NETWORK */
/////////////

void MapView::connectingToServer() {

    //when self user send
    QObject::connect(
        _rpzClient, &RPZClient::gameSessionReceived,
        this, &MapView::_onGameSessionReceived
    );

}
void MapView::_onGameSessionReceived(const RPZGameSession &gameSession) {
    
    Q_UNUSED(gameSession)

    //self
    auto self = this->_rpzClient->identity();

    //if host
    if(self.role() == RPZUser::Role::Host) {

        //send map history if host
        this->_sendMapHistory();

        //might update tokens when user updates/log in
        QObject::connect(
            this->_rpzClient, &RPZClient::userDataChanged,
            HintThread::hint(), &MapHint::mightUpdateOwnedTokens
        );
        QObject::connect(
            this->_rpzClient, &RPZClient::userJoinedServer,
            HintThread::hint(), &MapHint::mightUpdateOwnedTokens
        );


    }

    //if host
    bool is_remote = HintThread::hint()->ackRemoteness(
        self, 
        this->_rpzClient->getConnectedSocketAddress()
    );

    emit remoteChanged(is_remote);
    
}

void MapView::connectionClosed(bool hasInitialMapLoaded) {

    //reset impersonating character
    HintThread::hint()->defineImpersonatingCharacter();

    //back to default state
    if(hasInitialMapLoaded) QMetaObject::invokeMethod(HintThread::hint(), "loadDefaultRPZMap");

    emit remoteChanged(false);

}

void MapView::_sendMapHistory() {
    
    //generate payload
    auto payload = HintThread::hint()->generateResetPayload();
   
   //send it
    QMetaObject::invokeMethod(this->_rpzClient, "sendMapHistory", 
        Q_ARG(ResetPayload, payload)
    );

}

/////////////////
/* END NETWORK */
/////////////////


//////////
/* TOOL */
//////////

//returns tool
MapTool MapView::_getCurrentTool() const {
    return this->_quickTool == MapTool::Default ? this->_tool : this->_quickTool;
}

void MapView::_resetTool() {
    this->_quickTool = MapTool::Default;
    this->_changeTool(MapTool::Default);
}

void MapView::_changeTool(MapTool newTool, const bool quickChange) {

    //end drawing if any
    this->_drawingAssist->mayCommitDrawing();

    //prevent the usage of Atom tool if not host able
    if(!Authorisations::isHostAble() && newTool == MapTool::Atom) return;

    //if quick change asked
    if(quickChange) {

        this->_quickTool = newTool;

        //if unselecting quicktool
        if(newTool == MapTool::Default) newTool = this->_tool;

    } 
    
    //if standard tool change
    else {

        //since clearSelection wont trigger notification, hard call notification on reset
        if(this->_tool == MapTool::Atom && newTool == MapTool::Default) {
            this->_notifySelection();
        }

        this->_tool = newTool;
        
        if(newTool != MapTool::Walking) {
            this->scene()->clearSelection();
        }
        
    }    

    //if a quicktool is selected
    if(this->_quickTool != MapTool::Default) {
        newTool = this->_quickTool;
    }
    
    //destroy walking helper
    if(this->_tool != MapTool::Walking && this->_walkingHelper) {
        this->_clearWalkingHelper();
    }


    //depending on tool
    switch(newTool) {

        case MapTool::Atom: {
            
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);

            if(auto ghost = HintThread::hint()->ghostItem()) {
                ghost->setVisible(this->_isCursorIn);
            }
            
            switch(HintThread::hint()->templateAtom().type()) {
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

        }
        break;

        case MapTool::Scroll: {
            
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
            
            if(auto ghost = HintThread::hint()->ghostItem()) {
                ghost->setVisible(false); //force hidden if any
            
            }
        }
        break;
        
        case MapTool::Walking:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(this->_walkingCursor);
            break;

        case MapTool::Default:
        default:
            this->setInteractive(true);
            this->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
            this->setCursor(Qt::ArrowCursor);
            break;
    }

    //may update ghost pos
    this->_mightCenterGhostWithCursor();

}

void MapView::_clearWalkingHelper() {
    this->_toWalk = nullptr;
    delete this->_walkingHelper;
    this->_walkingHelper = nullptr;
}

//on received action
void MapView::onActionRequested(const MapAction &action) {
    
    switch(action) {
        
        case MapAction::ResetView:
            this->goToDefaultViewState();
            break;

        case MapAction::ResetTool:
            this->_resetTool();
            break;

        default:
            break;
            
    }

}

void MapView::onHelperActionTriggered(QAction *action) {
    this->setForegroundBrush(Qt::NoBrush); //force foreground re-drawing
}


void MapView::_mightCenterGhostWithCursor() {
    
    //update ghost item position relative to cursor
    if(auto ghost = this->_displayableGhostItem()) {
        
        //map cursor pos to widget
        auto cursorPos = this->mapFromGlobal(QCursor::pos());
        auto ghostCenter = ghost->boundingRect().center();
        QPointF cursorPosInScene;
        
        //check if ignores transformation
        if(ghost->flags().testFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations)) {
            cursorPos -= ghostCenter.toPoint();
            cursorPosInScene = this->mapToScene(cursorPos);
        } else {
            cursorPosInScene = this->mapToScene(cursorPos);
            cursorPosInScene -= ghostCenter;
        }

        //if grid movement and alignable, stick to grid
        if(this->_currentMapParameters.movementSystem() == RPZMapParameters::MovementSystem::Grid && RPZQVariant::isGridBound(ghost)) {
            this->_currentMapParameters.alignPointToGrid(cursorPosInScene);
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