#include "MapView.h"

MapView::MapView(QWidget *parent) : 
    QGraphicsView(parent),
    _hiddingBrush(new QBrush("#EEE", Qt::BrushStyle::SolidPattern)),
    _hints(new MapHint),
    _menuHandler(new AtomsContextualMenuHandler(_hints, this)) {

    this->_hints->moveToThread(new QThread);
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
}

MapView::~MapView() {
    if(this->_hints) {
        this->_hints->thread()->quit();
        this->_hints->thread()->wait();
    }
}


void MapView::onItemChanged(GraphicsItemsChangeNotifier* item, MapViewCustomItemsEventFlag flag) {

    switch(flag) {
        case MapViewCustomItemsEventFlag::Moved: {

            //add to list for future information
            this->_itemsWhoNotifiedMovement.insert(item->graphicsItem());

            //disable further notifications until information have been handled
            item->disableNotifications();

        }
        break;
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

        //if movement
        if(param == Position) {

            //enable notifications back on those items
            if(auto notifier = dynamic_cast<GraphicsItemsChangeNotifier*>(item)) {
                notifier->activateNotifications();
            }

            //remove from inner list
            this->_itemsWhoNotifiedMovement.remove(item);
        }

    }
}

void MapView::_handleHintsSignalsAndSlots() {

    //on map loading, set placeholder...
    QObject::connect(
        this->_hints, &AtomsStorage::heavyAlterationProcessing,
        this, &MapView::_displayLoader
    );

    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        this, &MapView::_onSceneSelectionChanged
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
    for(auto item : toUpdate) {
        if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
            auto c_pen = pathItem->pen();
            c_pen.setColor(newUser.color());
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
                this->_addItem(item, true);
            }
            break;

            case PA_Focused: {
                this->centerOn(item);
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

            default:
            break;
        }
    }

    this->_hideLoader();
}

void MapView::_addItem(QGraphicsItem* toAdd, bool mustNotifyMovement) {
    
    if(mustNotifyMovement) {
        if(auto notifier = dynamic_cast<GraphicsItemsChangeNotifier*>(toAdd)) {
            notifier->addNotified(this);
            notifier->activateNotifications();
        }
    }

    this->scene()->addItem(toAdd);

}

void MapView::contextMenuEvent(QContextMenuEvent *event) {

    auto pos = event->pos();
    auto selected = this->scene()->selectedItems();
    
    auto count = selected.count();

    //targets
    auto firstPass = true;
    auto riseLayoutTarget = 0;
    auto lowerLayoutTarget = 0;

    for(auto &item : selected) {
        
        auto atom = (RPZAtom*)item->data(RPZUserRoles::AtomPtr).toLongLong();
        auto layer = atom->layer();
        
        if(firstPass) {
            firstPass = false;
            riseLayoutTarget = layer;
            lowerLayoutTarget = layer;
            continue;
        }

        if(layer > riseLayoutTarget) riseLayoutTarget = layer;
        if(layer < lowerLayoutTarget) lowerLayoutTarget = layer;
    }

    riseLayoutTarget++;
    lowerLayoutTarget--;

    //create menu
    this->_menuHandler->invokeMenu(riseLayoutTarget, lowerLayoutTarget, count, this->viewport()->mapToGlobal(pos));
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

void MapView::keyPressEvent(QKeyEvent * event) {
    
    QGraphicsView::keyPressEvent(event);

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
            this->_animatedMove(Qt::Orientation::Vertical, -10);
            break;
        
        case Qt::Key::Key_Down:
            this->_animatedMove(Qt::Orientation::Vertical, 10);
            break;

        case Qt::Key::Key_Left:
            this->_animatedMove(Qt::Orientation::Horizontal, -10);
            break;

        case Qt::Key::Key_Right:
            this->_animatedMove(Qt::Orientation::Horizontal, 10);
            break;

    }

}

void MapView::assetTemplateChanged(const RPZAssetMetadata &assetMetadata) {
    this->_bufferedAssetMetadata = assetMetadata;
    this->_changeTool(assetMetadata.isEmpty() ? Tool::Default : Tool::Atom, false, true);
}

void MapView::_clearGhostItem() {
    if(!this->_ghostItem) return;
    this->scene()->removeItem(this->_ghostItem);
    delete this->_ghostItem;
    this->_ghostItem = nullptr;
}

void MapView::enterEvent(QEvent *event) {
    if(!this->_ghostItem) return;
    this->_ghostItem->setVisible(true);
}

void MapView::leaveEvent(QEvent *event) {
    if(!this->_ghostItem) return;

    //special case for keeping visible when right clicking
    auto cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
    if(this->geometry().contains(cursorPosInWidget)) return;

    this->_ghostItem->setVisible(false);
}

void MapView::onAtomTemplateChange() {
    QVector<const RPZAtom*> subjects {this->_hints->templateAtom()};
    emit subjectedAtomsChanged(subjects);
    
    if(this->_ghostItem) {
        //update the ghost graphics item to display the updated values
        AtomConverter::updateGraphicsItemFromAtom(
            this->_ghostItem, 
            *this->_hints->templateAtom(), 
            true
        );
    }

}

void MapView::_generateGhostItemFromBuffer() {
    this->_clearGhostItem();
    this->_ghostItem = this->_hints->generateGhostItem(this->_bufferedAssetMetadata);
    this->_addItem(this->_ghostItem);
}

void MapView::_handleGhostItem(const Tool &tool) {
    if(tool == Atom) this->_generateGhostItemFromBuffer();
    else {
        this->_clearGhostItem();
        auto subjects = QVector<const RPZAtom*>(); //no more subjects
        emit subjectedAtomsChanged(subjects);
    }
}


void MapView::_onSceneSelectionChanged() {
    this->_hints->notifySelectedItems(this->scene()->selectedItems());

        
    // if(type == PA_Selected) {
    //     auto selectedAtoms = this->_hints->selectedAtoms();
    //     emit subjectedAtomsChanged(selectedAtoms);
    // }

}

/////////////
/* NETWORK */
/////////////

void MapView::onRPZClientConnecting() {

    //save current map
    this->_hints->mayWantToSavePendingState();

    //when self user send
    QObject::connect(
        _rpzClient, &RPZClient::ackIdentity,
        this, &MapView::_onIdentityReceived
    );

    //when missing assets
    QObject::connect(
        this->_hints, &MapHint::requestMissingAssets,
        _rpzClient, &RPZClient::askForAssets
    );

    //when been asked for map content
    QObject::connect(
        _rpzClient, &RPZClient::beenAskedForMapHistory,
        this, &MapView::_sendMapHistory
    );

}
void MapView::_onIdentityReceived(const RPZUser &self) {
    this->_hints->setDefaultUser(self);

    //if host
    auto descriptor = self.role() == RPZUser::Role::Host ? NULL : this->_rpzClient->getConnectedSocketAddress();
    bool is_remote = this->_hints->defineAsRemote(descriptor);

    emit remoteChanged(is_remote);
}

void MapView::onRPZClientDisconnect() {

    //back to default state
    this->_hints->defineAsRemote();
    this->_hints->loadDefaultRPZMap();

}

void MapView::_sendMapHistory() {
    auto allAtoms = this->_hints->atoms();
    ResetPayload payload(allAtoms);
    QMetaObject::invokeMethod(this->_rpzClient, "sendMapHistory", Q_ARG(ResetPayload, payload));
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
                switch(this->_bufferedAssetMetadata.atomType()) {

                    case AtomType::Drawing:
                    case AtomType::Brush:
                        if(_stickyBrushIsDrawing) {
                            this->_savePosAsStickyNode(event->pos());
                        } else {
                            this->_beginDrawing(event->pos());
                        }
                    break;

                    default: {
                        this->_hints->integrateGraphicsItemAsPayload(this->_ghostItem);
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

    //make ghost item tracking mouse position
    if(this->_ghostItem) {
        this->_centerItemToPoint(this->_ghostItem, event->pos());
    }

    switch(this->_getCurrentTool()) {
        case Tool::Atom:
            switch(this->_bufferedAssetMetadata.atomType()) {
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
            if(this->_itemsWhoNotifiedMovement.count()) {

                //run notify
                this->_hints->notifyMovementOnItems(this->_itemsWhoNotifiedMovement.toList());

                //clear set
                this->_itemsWhoNotifiedMovement.clear();

            }
            
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
void MapView::_changeTool(Tool newTool, const bool quickChange, bool isFromExternal) {

    this->_endDrawing();

    //if quick change asked
    if(quickChange) {

        this->_quickTool = newTool;

        //if unselecting quicktool
        if(newTool == Tool::Default) newTool = this->_tool;

    } 
    
    //if standard tool change
    else {
        this->_tool = newTool;
        this->scene()->clearSelection();
    }    

    //if a quicktool is selected
    if(this->_quickTool != Tool::Default) {
        newTool = this->_quickTool;
    }

    //generate a ghost item if required
    this->_handleGhostItem(newTool);

    
    //depending on tool
    switch(newTool) {
        case Tool::Atom: {
            
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            
            switch(this->_bufferedAssetMetadata.atomType()) {
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
            if(!isFromExternal) emit unselectCurrentAssetAsked();
            break;
    }
}

//on received action
void MapView::actionRequested(const MapTools::Actions &action) {
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

void MapView::_animatedMove(const Qt::Orientation &orientation, int correction) {
    
    //prepare
    auto bar = orientation == Qt::Orientation::Vertical ? this->verticalScrollBar() : this->horizontalScrollBar();
    auto controller = orientation == Qt::Orientation::Vertical ? AnimationTimeLine::Type::VerticalMove : AnimationTimeLine::Type::HorizontalMove;

    //define animation handler
    AnimationTimeLine::use(controller, correction, this, [bar](qreal base, qreal prc) {
        bar->setValue(bar->value() + (int)base);
    });

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

    double zoomRatioToApply = event->delta() / 8;
    zoomRatioToApply = zoomRatioToApply / 15 / 20;

    //define animation handler
    auto zoom = [&](qreal base, qreal prc) {

        qreal factor = 1.0 + base;
        this->_currentRelScale = factor * this->_currentRelScale;
        this->scale(factor, factor);

        //update ghost item position relative to cursor
        if(this->_ghostItem) {
            this->_centerItemToPoint(this->_ghostItem, this->mapFromGlobal(QCursor::pos()));
        }

    };

    AnimationTimeLine::use(
        AnimationTimeLine::Type::Zoom, 
        zoomRatioToApply, 
        this, 
        zoom
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
    auto item = CustomGraphicsItemHelper::createGraphicsItem(
        *this->_hints->templateAtom(), 
        this->_bufferedAssetMetadata, 
        true
    );
    this->_addItem(item);
    this->_tempDrawing = static_cast<MapViewGraphicsPathItem*>(item);

    //determine if it must be sticky
    this->_stickyBrushIsDrawing = this->_hints->templateAtom()->brushType() == BrushType::Cutter;
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

    switch(this->_hints->templateAtom()->type()) {
        
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
    
    switch(this->_hints->templateAtom()->brushType()) {
        
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