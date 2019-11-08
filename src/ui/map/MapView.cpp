#include "MapView.h"

MapView::MapView(QWidget *parent) : QGraphicsView(parent), MV_Manipulation(this), MV_HUDLayout(this) {

    this->setScene(new QGraphicsScene); //dummy scene

    this->_walkingCursor = QCursor(QStringLiteral(u":/icons/app/tools/walking.png"));

    //init
    this->_hints = new MapHint;
    this->_menuHandler = new AtomsContextualMenuHandler(this->_hints, this);
    this->_atomActionsHandler = new AtomActionsHandler(this->_hints, this, this);
    this->_drawingAssist = new DrawingAssist(this->_hints, this);

    //OpenGL backend activation
    QGLFormat format;
    format.setSampleBuffers(true);
    format.setDirectRendering(true);
    format.setAlpha(true);
    this->setViewport(new QGLWidget(format));

    //hide scrollbars
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    this->setRubberBandSelectionMode(Qt::ItemSelectionMode::ContainsItemBoundingRect); //rubberband UC optimization
    this->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::SmartViewportUpdate); //force viewport update mode
    this->setMouseTracking(true); //activate mouse tracking for ghost 

    //thread
    this->_hints->moveToThread(new QThread);
    this->_hints->thread()->setObjectName(QStringLiteral(u"MapThread"));
    this->_hints->thread()->start();

    this->_resetTool();

    this->_handleHintsSignalsAndSlots();

}

MapView::~MapView() {
    if(this->_hints) {
        this->_hints->thread()->quit();
        this->_hints->thread()->wait();
    }
}

MapHint* MapView::hints() const {
    return this->_hints;
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
        [=]() {this->endHeavyLoadPlaceholder();}
    );

    QObject::connect(
        this->_hints, &ViewMapHint::requestingUIAlteration,
        this, &MapView::_onUIAlterationRequest
    );
    
    QObject::connect(
        this->_hints, QOverload<const QHash<QGraphicsItem*, RPZAtom::Updates>&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QHash<QGraphicsItem*, RPZAtom::Updates>&>::of(&MapView::_onUIUpdateRequest)
    );
    
    QObject::connect(
        this->_hints, QOverload<const QList<QGraphicsItem*>&, const RPZAtom::Updates&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QList<QGraphicsItem*>&, const RPZAtom::Updates&>::of(&MapView::_onUIUpdateRequest)
    );

    QObject::connect(
        this->_hints, &AtomsStorage::mapParametersChanged,
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

//
//
//

void MapView::_notifySelection() {
    this->_hints->notifySelectedItems(
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
}

void MapView::_onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates) {
    for(auto item : toUpdate) {
        this->_updateItemValue(item, updates);
    }
}

void MapView::_onUIAlterationRequest(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter) {
    
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

        //setup loader
        this->setupHeavyLoadPlaceholder(toAlter.count());

    }

    auto currentTool = this->_getCurrentTool();

    for(auto item : toAlter) {
        switch(type) {

            case Payload::Alteration::Reset: {
                this->scene()->addItem(item);        
                this->incrementHeavyLoadPlaceholder();
            }
            break;

            case Payload::Alteration::Added: {
                this->scene()->addItem(item);
                
                //auto remove temporary drawing
                auto isCommitedDrawing = this->_drawingAssist->compareItemToCommitedDrawing(item);
                
                //if not from temporary drawing, animate path
                if(!isCommitedDrawing) {
                    if(auto canBeAnimated = dynamic_cast<MapViewGraphicsPathItem*>(item)) {
                        MapViewAnimator::animatePath(canBeAnimated, canBeAnimated->path());
                    } 
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
                auto selectedAtom = item ? this->_hints->templateAtom() : RPZAtom();
                if(item) this->scene()->addItem(item);

                //change tool, proceed to unselect items
                this->_changeTool(newTool);

                //emit that template selection has changed
                AtomTemplateSelectedPayload payload(selectedAtom);
                AlterationHandler::get()->queueAlteration(this->_hints, payload);
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
        
        auto result = this->_hints->latestEligibleCharacterIdOnSelection();
        auto can = RPZClient::isHostAble() && result.first;
        //TODO restrict to player only
        
        if(can) {
            
            this->_clearWalkingHelper();

            this->_toWalk = toAlter.first();
            this->_walkingHelper = new MapViewWalkingHelper(this->_currentMapParameters, this->_toWalk, this);
            this->scene()->addItem(this->_walkingHelper);
            this->_changeTool(MapTool::Walking);
            
        }

    }

}

//
//
//


//////////////////
// UI rendering //
//////////////////

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

    //check item
    auto item = this->itemAt(event->pos());
    if(!item) return;

    //check item is not temporary !
    if(AtomConverter::isTemporary(item)) return;

    //notify focus
    this->_hints->notifyFocusedItem(item);
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
    return this->_hints->getAtomIdsFromGraphicsItems(
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
    return this->_hints->ghostItem();
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
                    this->_ignoreSelectionChangedEvents = true;
                }
                break;

                case MapTool::Walking: {
                    auto scenePos = this->mapToScene(event->pos());
                    this->_hints->notifyWalk(this->_toWalk, scenePos);
                    MapViewAnimator::animateMove(this->_walkingHelper, scenePos);
                }
                break;

                case MapTool::Atom: {
                    
                    //check if inserts are allowed
                    if(!RPZClient::isHostAble()) break;

                    //conditionnal drawing
                    switch(this->_hints->templateAtom().type()) {

                        case RPZAtom::Type::Drawing:
                        case RPZAtom::Type::Brush:
                            this->_drawingAssist->addDrawingPoint(event->pos());
                        break;

                        default: {
                            this->_hints->integrateGraphicsItemAsPayload(
                                this->_hints->ghostItem()
                            );
                        }
                        break;

                    }

                }
                break;

                default:
                break;

            }

        }
        break;

        default:
            break;
    }

    QGraphicsView::mousePressEvent(event);
}

//on movement
void MapView::mouseMoveEvent(QMouseEvent *event) {

    this->_mightCenterGhostWithCursor();

    auto currentTool = this->_getCurrentTool();

    if(currentTool == MapTool::Atom) {
        
        auto type = this->_hints->templateAtom().type();
        
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
                    this->_hints->mightNotifyMovement(this->scene()->selectedItems()); 

                    //trigger items selection
                    QGraphicsView::mouseReleaseEvent(event);

                    //update selection
                    this->_notifySelection();
                    this->_ignoreSelectionChangedEvents = false;

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

void MapView::wheelEvent(QWheelEvent *event) {

    //make sure no button is pressed
    if(this->_isMousePressed) return;
    
    this->animateScroll(event, this->_currentMapParameters);

};

//////////////////////
/* END MOUSE EVENTS */
//////////////////////

/////////////
/* NETWORK */
/////////////

void MapView::connectingToServer() {

    //when self user send
    QObject::connect(
        _rpzClient, &RPZClient::selfIdentityAcked,
        this, &MapView::_onIdentityReceived
    );

}
void MapView::_onIdentityReceived(const RPZUser &self) {
    
    //send map history if host
    if(self.role() == RPZUser::Role::Host) {
        this->_sendMapHistory();
    }

    //if host
    bool is_remote = this->_hints->ackRemoteness(self, _rpzClient);

    emit remoteChanged(is_remote);
    
}

void MapView::connectionClosed(bool hasInitialMapLoaded) {

    //back to default state
    if(hasInitialMapLoaded) QMetaObject::invokeMethod(this->_hints, "loadDefaultRPZMap");

    emit remoteChanged(false);

}

void MapView::_sendMapHistory() {
    
    //generate payload
    auto payload = this->_hints->generateResetPayload();
   
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

            if(auto ghost = this->_hints->ghostItem()) {
                ghost->setVisible(this->_isCursorIn);
            }
            
            switch(this->_hints->templateAtom().type()) {
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
            
            if(auto ghost = this->_hints->ghostItem()) {
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
        if(this->_currentMapParameters.movementSystem() == RPZMapParameters::MovementSystem::Grid && RPZQVariant::isAlignableOnGrid(ghost)) {
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