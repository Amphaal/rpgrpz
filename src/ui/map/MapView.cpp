#include "MapView.h"

MapView::MapView(QWidget *parent) : QGraphicsView(parent),
    _hiddingBrush(new QBrush("#EEE", Qt::BrushStyle::SolidPattern)),
    _hints(new MapHint),
    _menuHandler(new AtomsContextualMenuHandler(_hints, this)) {

    //hide scrollbars
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //thread
    this->_hints->moveToThread(new QThread);
    this->_hints->thread()->setObjectName("MapThread");
    this->_hints->thread()->start();

    //default
    auto scene = new QGraphicsScene(
        this->_defaultSceneSize, 
        this->_defaultSceneSize, 
        this->_defaultSceneSize, 
        this->_defaultSceneSize
    );
    this->setScene(scene);
    this->_resetTool();
    
    //openGL activation
    QGLFormat format;
    format.setSampleBuffers(true);
    format.setDirectRendering(true);
    format.setAlpha(true);
    this->setViewport(new QGLWidget(format));
    this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->update();

    //rubberband UC optimization
    this->setRubberBandSelectionMode(Qt::ItemSelectionMode::ContainsItemBoundingRect);

    //background / foreground
    this->setBackgroundBrush(QBrush("#EEE", Qt::BrushStyle::CrossPattern));

    this->_handleHintsSignalsAndSlots();

    //default state
    this->scale(this->_defaultScale, this->_defaultScale);
    this->_goToDefaultViewState();

    //activate mouse tracking for ghost 
    this->setMouseTracking(true);

    //animator configuration 
    this->_configureMoveAnimator();

}

MapView::~MapView() {
    if(this->_hints) {
        this->_hints->thread()->quit();
        this->_hints->thread()->wait();
    }
}


void MapView::_updateItemValue(QGraphicsItem* item, const AtomUpdates &updates) {
    
    for(auto i = updates.constBegin(); i != updates.constEnd(); ++i) {
    
        auto param = i.key();

        //update GI
        AtomConverter::updateGraphicsItemFromMetadata(
            item,
            param,
            i.value()
        );

    }
}

void MapView::_handleHintsSignalsAndSlots() {

    //on map loading, set placeholder...
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
        this, &MapView::_displayLoader
    );

    //define selection debouncer
    this->_debounceSelection.setInterval(100);
    this->_debounceSelection.setSingleShot(true);
    this->_debounceSelection.callOnTimeout([=]() {
        this->_hints->notifySelectedItems(
            this->scene()->selectedItems()
        );
    });

    //call debouncer on selection
    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        [=]() {this->_debounceSelection.start();}
    );

    QObject::connect(
        this->_hints, &ViewMapHint::requestingUIAlteration,
        this, &MapView::_onUIAlterationRequest
    );
    
    QObject::connect(
        this->_hints, QOverload<const QHash<QGraphicsItem*, AtomUpdates>&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QHash<QGraphicsItem*, AtomUpdates>&>::of(&MapView::_onUIUpdateRequest)
    );
    
    QObject::connect(
        this->_hints, QOverload<const QList<QGraphicsItem*>&, const AtomUpdates&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QList<QGraphicsItem*>&, const AtomUpdates&>::of(&MapView::_onUIUpdateRequest)
    );

    QObject::connect(
        this->_hints, &ViewMapHint::requestingUIUserChange,
        this, &MapView::_onUIUserChangeRequest
    );

}

void MapView::_hideLoader() {
    if(!this->_isLoading) return;
    this->setForegroundBrush(QBrush());
    this->_isLoading = false;
}

void MapView::_displayLoader() {
    this->_isLoading = true;
    this->setForegroundBrush(*this->_hiddingBrush);
}

void MapView::_onUIUpdateRequest(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate) {
    
    for(auto i = toUpdate.constBegin(); i != toUpdate.constEnd(); i++) {
        this->_updateItemValue(i.key(), i.value());
    }
    
    this->_hideLoader();
}

void MapView::_onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates) {
    for(auto item : toUpdate) {
        this->_updateItemValue(item, updates);
    }
    this->_hideLoader();
}

void MapView::_onUIUserChangeRequest(const QList<QGraphicsItem*> &toUpdate, const RPZUser &newUser) {
    
    auto newColor = newUser.color();
    
    for(auto item : toUpdate) {
        if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
            auto c_pen = pathItem->pen();
            c_pen.setColor(newColor);
            pathItem->setPen(c_pen);
        } 
    }

    this->_hideLoader();
    
}

void MapView::_onUIAlterationRequest(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter) {
    
    //prevent circual selection
    QSignalBlocker b(this->scene());

    if(type == PA_Selected) this->scene()->clearSelection();
    if(type == PA_Reset) this->scene()->clear();

    for(auto item : toAlter) {
        switch(type) {

            case PA_Reset:
            case PA_Added: {
                this->_addItem(item);
            }
            break;

            case PA_Focused: {
                this->_focusItem(item);
            }
            break;

            case PA_Selected: {
                item->setSelected(true);
            }
            break;

            case PA_Removed: {
                this->scene()->removeItem(item);
                delete item;
            }
            break;

            case PA_AssetSelected: {
                Tool newTool = item ? Tool::Atom : Tool::Default;
                auto selectedAtom = item ? this->_hints->templateAtom() : RPZAtom();
                if(item) this->_addItem(item);

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

    this->_hideLoader();
}

QRectF MapView::_getVisibleRect() {
    return mapToScene(viewport()->rect()).boundingRect();
}

void MapView::_focusItem(QGraphicsItem* toFocus) {
    
    this->centerOn(toFocus);
    
    auto bound = toFocus->sceneBoundingRect();
    bound = bound.marginsAdded(
        QMarginsF(
            bound.width() / 2,
            bound.height() / 2,
            bound.width() / 2,
            bound.height() / 2
        )
    );
    
    this->fitInView(bound, Qt::AspectRatioMode::KeepAspectRatio);

}

void MapView::_addItem(QGraphicsItem* toAdd) {
    this->scene()->addItem(toAdd);
}

void MapView::contextMenuEvent(QContextMenuEvent *event) {

    auto pos = this->viewport()->mapToGlobal(
        event->pos()
    );
    auto ids = this->_hints->getAtomIdsFromGraphicsItems(
        this->scene()->selectedItems()
    );

    //create menu
    this->_menuHandler->invokeMenu(ids, pos);

}

void MapView::mouseDoubleClickEvent(QMouseEvent *event) {
    
    //check item
    auto item = this->itemAt(event->pos());
    if(!item) return;

    //check item is not temporary !
    auto isTemporary = item->data((int)AtomConverterDataIndex::IsTemporary).toBool();
    if(isTemporary) return;

    //notify focus
    this->_hints->notifyFocusedItem(item);
}

void MapView::resizeEvent(QResizeEvent * event) {
    this->_goToSceneCenter();
}

void MapView::_goToDefaultViewState() {
    this->_goToSceneCenter();
    this->_goToDefaultZoom();
}

MapHint* MapView::hints() const {
    return this->_hints;
}

void MapView::keyReleaseEvent(QKeyEvent *event) {

    switch(event->key()) {
        case Qt::Key::Key_Up:
            event->ignore();
            return this->_removeAnimatedMove(GoUp);
            break;
        
        case Qt::Key::Key_Down:
            event->ignore();
            return this->_removeAnimatedMove(GoDown);
            break;

        case Qt::Key::Key_Left:
            event->ignore();
            return this->_removeAnimatedMove(GoLeft);
            break;

        case Qt::Key::Key_Right:
            event->ignore();
            return this->_removeAnimatedMove(GoRight);
            break;
    }

    QGraphicsView::keyReleaseEvent(event);

}

void MapView::keyPressEvent(QKeyEvent * event) {

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->_hints->deleteCurrentSelectionItems();
            break;
        
        //ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->_resetTool();
            break;
        
        case Qt::Key::Key_Up:
            event->ignore();
            return this->_addAnimatedMove(GoUp);
            break;
        
        case Qt::Key::Key_Down:
            event->ignore();
            return this->_addAnimatedMove(GoDown);
            break;

        case Qt::Key::Key_Left:
            event->ignore();
            return this->_addAnimatedMove(GoLeft);
            break;

        case Qt::Key::Key_Right:
            event->ignore();
            return this->_addAnimatedMove(GoRight);
            break;

    }

    QGraphicsView::keyPressEvent(event);

}

void MapView::_mightCenterGhostWithCursor() {
    //update ghost item position relative to cursor
    if(auto ghost = this->_hints->ghostItem()) {
        this->_centerItemToPoint(ghost, this->mapFromGlobal(QCursor::pos()));
    }
}

void MapView::enterEvent(QEvent *event) {
    if(auto ghost = this->_hints->ghostItem()) {
        ghost->setVisible(true);
    }  
}

void MapView::leaveEvent(QEvent *event) {
    if(auto ghost = this->_hints->ghostItem()) {

        //special case for keeping visible when right clicking
        auto cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
        if(this->geometry().contains(cursorPosInWidget)) return;

        ghost->setVisible(false);

    }
}

/////////////
/* NETWORK */
/////////////

void MapView::onRPZClientConnecting() {

    //when self user send
    QObject::connect(
        _rpzClient, &RPZClient::ackIdentity,
        this, &MapView::_onIdentityReceived
    );

}
void MapView::_onIdentityReceived(const RPZUser &self) {
    
    //send map history if host
    if(self.role() == RPZUser::Role::Host) {
        this->_sendMapHistory();
    }

    //define default creation user
    this->_hints->setDefaultUser(self);

    //if host
    auto descriptor = self.role() == RPZUser::Role::Host ? NULL : this->_rpzClient->getConnectedSocketAddress();
    bool is_remote = this->_hints->defineAsRemote(descriptor);

    emit remoteChanged(is_remote);
    
}

void MapView::onRPZClientDisconnect() {

    //back to default state
    QMetaObject::invokeMethod(this->_hints, "loadDefaultRPZMap");

}

void MapView::_sendMapHistory() {
    auto payload = this->_hints->createStatePayload();
    QMetaObject::invokeMethod(this->_rpzClient, "sendMapHistory", 
        Q_ARG(ResetPayload, payload)
    );
}

/////////////////
/* END NETWORK */
/////////////////

//////////////////
/* MOUSE EVENTS */
//////////////////

//mouse click
void MapView::mousePressEvent(QMouseEvent *event) {

    //register last position
    this->_isMousePressed = true;

    switch(event->button()) {

        case Qt::MouseButton::MiddleButton: {
            auto tool = this->_quickTool == Default ? Tool::Scroll : Tool::Default;
            this->_changeTool(tool, true);
        }
        break;

        case Qt::MouseButton::LeftButton: {

            if(this->_getCurrentTool() == Atom) {
                switch(this->_hints->templateAtom().type()) {

                    case AtomType::Drawing:
                    case AtomType::Brush:
                        if(_stickyBrushIsDrawing) {
                            this->_savePosAsStickyNode(event->pos());
                        } else {
                            this->_beginDrawing(event->pos());
                        }
                    break;

                    default: {
                        this->_hints->integrateGraphicsItemAsPayload(this->_hints->ghostItem());
                    }
                    break;

                }
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

    switch(this->_getCurrentTool()) {
        case Tool::Atom:
            switch(this->_hints->templateAtom().type()) {
                case AtomType::Drawing:
                case AtomType::Brush:
                    this->_updateDrawingPath(event->pos());
                break;

                default:
                    break;
            }
        
        default:
            break;
    }

    QGraphicsView::mouseMoveEvent(event);
}

//mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {

    switch(event->button()) {
        case Qt::MouseButton::LeftButton: {
            
            //if was drawing...
            if(!this->_stickyBrushIsDrawing) this->_endDrawing();

            //if something moved ?
            this->_hints->mightNotifyMovement(this->scene()->selectedItems());
            
        }
        break;

        default:
            break;
    }

    this->_isMousePressed = false;

    QGraphicsView::mouseReleaseEvent(event);
}

//////////////////////
/* END MOUSE EVENTS */
//////////////////////

//////////
/* TOOL */
//////////

//returns tool
MapView::Tool MapView::_getCurrentTool() const {
    return this->_quickTool == Tool::Default ? this->_tool : this->_quickTool;
}

void MapView::_resetTool() {
    this->_quickTool = Tool::Default;
    this->_changeTool(Tool::Default);
}

//change tool
void MapView::_changeTool(Tool newTool, const bool quickChange) {

    this->_endDrawing();

    //if quick change asked
    if(quickChange) {

        this->_quickTool = newTool;

        //if unselecting quicktool
        if(newTool == Tool::Default) newTool = this->_tool;

    } 
    
    //if standard tool change
    else {

        //since clearSelection wont trigger notification, hard call notification on reset
        if(this->_tool == Atom && newTool == Default) {
            this->_hints->notifySelectedItems(
                this->scene()->selectedItems()
            );
        }

        this->_tool = newTool;
        this->scene()->clearSelection();
    }    

    //if a quicktool is selected
    if(this->_quickTool != Tool::Default) {
        newTool = this->_quickTool;
    }
    
    //depending on tool
    switch(newTool) {
        case Tool::Atom: {
            
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            
            switch(this->_hints->templateAtom().type()) {
                case AtomType::Drawing:
                    this->setCursor(Qt::CrossCursor);
                    break;
                case AtomType::Object:
                    this->setCursor(Qt::ClosedHandCursor);
                    break;
                case AtomType::Text:
                    this->setCursor(Qt::IBeamCursor);
                    break;
                default:
                    this->setCursor(Qt::CrossCursor);
                    break;
            }

        }
        break;
        case Tool::Scroll:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
            break;
        case Tool::Default:
        default:
            this->setInteractive(true);
            this->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
            this->setCursor(Qt::ArrowCursor);
            break;
    }
}

//on received action
void MapView::onActionRequested(const MapTools::Actions &action) {
    switch(action) {
        case MapTools::Actions::ResetView:
            this->_goToDefaultViewState();
            break;
        case MapTools::Actions::ResetTool:
            this->_resetTool();
            break;
        default:
            break;
    }
}

//////////////
/* END TOOL */
//////////////

//////////
/* MOVE */
//////////

void MapView::_goToSceneCenter() {
    auto center = this->scene()->sceneRect().center();
    this->centerOn(center);
}

void MapView::_addAnimatedMove(const MapView::MoveDirection &direction) {

    //set direction, remove opposite
    this->_currentMoveDirections.insert(direction);
    auto opposite = this->_getOppositeDirection(direction);
    this->_currentMoveDirections.remove(opposite);

    //update instructions
    this->_currentMoveInstructions = this->_getMoveInstructions(this->_currentMoveDirections);

    //start if necessary
    this->_moveAnimator.stop();
    this->_stiffMove.start();

}

void MapView::_removeAnimatedMove(const MapView::MoveDirection &direction) {
    
    //remove direction
    this->_currentMoveDirections.remove(direction);

    //update instructions
    if(this->_currentMoveDirections.isEmpty()) {
        this->_moveAnimator.start();
        this->_stiffMove.stop();
    } else {
        this->_currentMoveInstructions = this->_getMoveInstructions(this->_currentMoveDirections);
    }
    
}

void MapView::_configureMoveAnimator() {
    
    //define animation
    auto animation = [=](qreal x = 1) {
            
        for(auto &inst : this->_currentMoveInstructions) {
            
            //loss acceptable
            auto castedDelta = (int)(inst.correction * 5 * x);

            //update scrollbar
            inst.affectedScroll->setValue(
                inst.affectedScroll->value() + castedDelta
            );

        }
        
        this->_mightCenterGhostWithCursor();

    };

    //define stiff animation
    this->_stiffMove.setInterval(10);
    QObject::connect(
        &this->_stiffMove, &QTimer::timeout,
        animation
    );
    
    //define
    this->_moveAnimator.setDuration(300);
    this->_moveAnimator.setUpdateInterval(10);
    this->_moveAnimator.setDirection(QTimeLine::Direction::Backward);
    this->_moveAnimator.setCurveShape(QTimeLine::CurveShape::EaseOutCurve);

    //on finished, reset any move
    QObject::connect(
        &this->_moveAnimator, &QTimeLine::finished,
        [=](){
            this->_currentMoveDirections.clear();
            this->_currentMoveInstructions.clear();
        }
    );

    //on animation going
    QObject::connect(
        &this->_moveAnimator, &QTimeLine::valueChanged,
        animation
    );
}


QList<MapView::MoveInstruction> MapView::_getMoveInstructions(const QSet<MapView::MoveDirection> &directions) {
    QList<MapView::MoveInstruction> out;

    for(auto &direction : directions) {
        
        MapView::MoveInstruction temp;

        switch(direction) {
            
            case GoLeft:
                temp.affectedScroll = this->horizontalScrollBar();
                temp.correction = -1;
            break;

            case GoUp:
                temp.affectedScroll = this->verticalScrollBar();
                temp.correction = -1;
            break;

            case GoRight:
                temp.affectedScroll = this->horizontalScrollBar();
                temp.correction = 1;
            break;

            case GoDown:
                temp.affectedScroll = this->verticalScrollBar();
                temp.correction = 1;
            break;

        }

        out += temp;

    }

    return out;
}

MapView::MoveDirection MapView::_getOppositeDirection(const MapView::MoveDirection &direction) {
    switch(direction) {
        case MapView::MoveDirection::GoLeft:
            return MapView::MoveDirection::GoRight;
        break;

        case MapView::MoveDirection::GoUp:
            return MapView::MoveDirection::GoDown;
        break;

        case MapView::MoveDirection::GoRight:
            return MapView::MoveDirection::GoLeft;
        break;

        case MapView::MoveDirection::GoDown:
            return MapView::MoveDirection::GoUp;
        break;

        default:
            return MapView::MoveDirection::GoUndefined;
        break;
    }
}


//////////////
/* END MOVE */
//////////////

//////////
/* ZOOM */
//////////

void MapView::_goToDefaultZoom() {
    auto corrected = 1/(this->_currentRelScale);
    this->scale(corrected, corrected);
    this->_currentRelScale = 1;
}

void MapView::wheelEvent(QWheelEvent *event) {

    //make sure no button is pressed
    if(this->_isMousePressed) return;

    //cap acceleration to 5% per tick
    auto delta = event->delta();
    auto modifier = ((double)delta / 5000);
    modifier = std::clamp(modifier, -.05, .05);

    //define animation handler
    AnimationTimeLine::use(
        AnimationTimeLine::Type::Zoom, 
        modifier, 
        [&](qreal base, qreal prc) {
            auto factor = 1.0 + (prc * base);
            this->_currentRelScale = factor * this->_currentRelScale;
            this->scale(factor, factor);
            this->_mightCenterGhostWithCursor();
        }
    );

};


//////////////
/* END ZOOM */
//////////////

/////////////
/* DRAWING */
/////////////

void MapView::_destroyTempDrawing() {
    if(!this->_tempDrawing) return;
    
    //remove helpers
    for(auto helper : this->_tempDrawingHelpers) {
        this->scene()->removeItem(helper);
        delete helper;
    }
    this->_tempDrawingHelpers.clear();
        
    //remove drawing
    this->scene()->removeItem(this->_tempDrawing);
    delete this->_tempDrawing;
    this->_tempDrawing = nullptr;

    //reset sticky
    if(this->_stickyBrushIsDrawing) {   
        this->_stickyBrushIsDrawing = false;
        this->_stickyBrushValidNodeCount = 0;
    }
    
}

void MapView::_beginDrawing(const QPoint &lastPointMousePressed) {

    //destroy temp
    this->_destroyTempDrawing();

    //create base and store it
    auto item = this->_hints->generateTemporaryItemFromTemplateBuffer();
    this->_addItem(item);
    this->_tempDrawing = static_cast<MapViewGraphicsPathItem*>(item);

    //determine if it must be sticky
    this->_stickyBrushIsDrawing = this->_hints->templateAtom().brushType() == BrushType::Cutter;
    this->_stickyBrushValidNodeCount = this->_stickyBrushIsDrawing ? this->_tempDrawing->path().elementCount() : 0;

    //update position
    this->_centerItemToPoint(this->_tempDrawing, lastPointMousePressed);

    //add outline
    if(this->_stickyBrushIsDrawing) {
        auto pos = this->_tempDrawing->pos();
        auto outline = CustomGraphicsItemHelper::createOutlineRectItem(pos);
        this->_addItem(outline);
        this->_tempDrawingHelpers.append(outline);
    }
}

void MapView::_updateDrawingPath(const QPoint &evtPoint) {
    
    //if no temp, stop
    if(!this->_tempDrawing) return;

    //get existing path
    auto existingPath = this->_tempDrawing->path();

    //define destination coordonate
    auto sceneCoord = this->mapToScene(evtPoint);
    auto pathCoord = this->_tempDrawing->mapFromScene(sceneCoord);

    switch(this->_hints->templateAtom().type()) {
        
        case AtomType::Drawing:
            existingPath.lineTo(pathCoord);
        break;

        case AtomType::Brush:
            this->_updateDrawingPathForBrush(pathCoord, existingPath, this->_tempDrawing);
        break;

        default:
            break;

    }

    //save as new path
    this->_tempDrawing->setPath(existingPath);
}

void MapView::_updateDrawingPathForBrush(const QPointF &pathCoord, QPainterPath &pathToAlter, MapViewGraphicsPathItem* sourceTemplate) {
    
    switch(this->_hints->templateAtom().brushType()) {
        
        case BrushType::Stamp: {
            
            //if contained in path, nothing to do
            if(pathToAlter.contains(pathCoord)) return;

            auto xRatio = qCeil(pathCoord.x() / sourceTemplate->sourceBrushSize().width()) - 1;
            auto yRatio = qCeil(pathCoord.y() / sourceTemplate->sourceBrushSize().height()) - 1;
            auto expectedStampPos = QPointF(
                xRatio * sourceTemplate->sourceBrushSize().width(),
                yRatio * sourceTemplate->sourceBrushSize().height()
            );

            //if something at expected, nothing to do
            if(pathToAlter.contains(expectedStampPos)) return;
            
            //add rect
            QRectF rect(expectedStampPos, sourceTemplate->sourceBrushSize());
            pathToAlter.addRect(rect);

        }
        break;

        case BrushType::Cutter: {
            auto count = pathToAlter.elementCount();
            
            //if no temporary node, create it
            if(this->_stickyBrushValidNodeCount == count) {
                pathToAlter.lineTo(pathCoord);
            } 
            
            //update temporary node
            else {
                pathToAlter.setElementPositionAt(count-1, pathCoord.x(), pathCoord.y());
            }
        }                
        break;

        case BrushType::Ovale: {
            pathToAlter = QPainterPath();
            QRectF rect(QPointF(0,0), pathCoord);
            pathToAlter.addEllipse(rect);
        }
        break;

        case BrushType::Rectangle: {
            pathToAlter = QPainterPath();
            QRectF rect(QPointF(0,0), pathCoord);
            pathToAlter.addRect(rect);
        }
        break;

        case BrushType::Scissors: {
            pathToAlter.lineTo(pathCoord);
        }
        break;

        case BrushType::RoundBrush: {
            pathToAlter.lineTo(pathCoord);
        }
        break;

        default:
            break;

    }
}

void MapView::_endDrawing() {
    
    //if no temporary drawing, stop
    if(!this->_tempDrawing) return;
    
    //if too small stop
    auto path = this->_tempDrawing->path();
    if(path.elementCount() < 2) return;

    //add definitive path
    this->_hints->integrateGraphicsItemAsPayload(this->_tempDrawing);

    //destroy temp
    this->_destroyTempDrawing();

}

void MapView::_savePosAsStickyNode(const QPoint &evtPoint) {
    
    //dest pos
    auto sceneCoord = this->mapToScene(evtPoint);
    auto destCoord = this->_tempDrawing->mapFromScene(sceneCoord);
    
    //Update
    auto path = this->_tempDrawing->path();
    auto count = path.elementCount();
    path.setElementPositionAt(count-1, destCoord.x(), destCoord.y());
    this->_tempDrawing->setPath(path);
    
    //add visual helper
    auto outline = CustomGraphicsItemHelper::createOutlineRectItem(sceneCoord);
    this->_addItem(outline);
    this->_tempDrawingHelpers.append(outline);

    //update
    this->_stickyBrushValidNodeCount = count;
    
    //check vicinity between last node and first node
    auto firstNode = this->_tempDrawing->mapToScene(path.elementAt(0));
    auto firstNodeGlobalPos = this->mapFromScene(firstNode);
    auto gap = evtPoint - firstNodeGlobalPos;

    //if gap is minimal, understand that the users wants to end the drawing
    if(gap.manhattanLength() < 20) {
        this->_endDrawing();
    }
}

/////////////////
/* END DRAWING */
/////////////////

void MapView::_centerItemToPoint(QGraphicsItem* item, const QPoint &eventPos) {
    QPointF point = this->mapToScene(eventPos);
    point = point - item->boundingRect().center();
    item->setPos(point);
}