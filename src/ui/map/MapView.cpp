#include "MapView.h"

MapView::MapView(QWidget *parent) : QGraphicsView(parent) {

    //default
    this->_scene = new MapViewGraphicsScene(this->_defaultSceneSize);
    this->setScene(this->_scene);
    this->_hints = new ViewMapHint(this); //after first inst of scene
    this->setAcceptDrops(true);
    this->_changeTool(MapView::_defaultTool);
    
    //openGL activation
    this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DirectRendering)));
    this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->update();

    //background / foreground
    this->setBackgroundBrush(QBrush("#EEE", Qt::BrushStyle::CrossPattern));

    //to route from MapHints
    QObject::connect(
        this->_hints, &AtomsStorage::alterationRequested,
        this, &MapView::_sendMapChanges
    );

    //default state
    this->scale(this->_defaultScale, this->_defaultScale);
    this->_goToDefaultViewState();
    
}

void MapView::contextMenuEvent(QContextMenuEvent *event) {

    auto pos = event->pos();
    auto selected = this->scene()->selectedItems();
    if(!selected.count()) return;

    //targets
    auto firstPass = true;
    auto riseLayoutTarget = 0;
    auto lowerLayoutTarget = 0;

    for(auto &item : selected) {
        
        auto atom = (RPZAtom*)item->data(0).toLongLong();
        auto layer = atom->metadata().layer();
        
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
    this->_hints->invokeMenu(riseLayoutTarget, lowerLayoutTarget, this->viewport()->mapToGlobal(pos));
}

void MapView::resizeEvent(QResizeEvent * event) {
    this->_goToSceneCenter();
}

void MapView::_goToDefaultViewState() {
    this->_goToSceneCenter();
    this->_goToDefaultZoom();
}

ViewMapHint* MapView::hints() {
    return this->_hints;
}

void MapView::keyPressEvent(QKeyEvent * event) {
    
    QGraphicsView::keyPressEvent(event);
    
    if(this->_hints->isInTextInteractiveMode()) return;

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->_hints->deleteCurrentSelectionItems();
            break;
        
        //ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->_changeTool(MapView::_defaultTool);
            emit unselectCurrentToolAsked();
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

void MapView::leaveEvent(QEvent *event) {
    this->_clearTempText();
}

void MapView::_clearTempText() {
    if(!this->_tempText) return;
    delete this->_tempText;
    this->_tempText = nullptr;
}

void MapView::enterEvent(QEvent *event) {
    this->_generateTempText();
}

void MapView::_generateTempText() {
    if(this->_getCurrentTool() == MapTools::Actions::Text) {
        if(this->_tempText) this->_clearTempText();
        this->_tempText = this->_hints->generateGhostTextItem();
    }
}

/////////////
/* NETWORK */
/////////////

void MapView::onRPZClientConnecting(RPZClient * cc) {

    ClientBindable::onRPZClientConnecting(cc);

    //save current map
    this->_hints->mayWantToSavePendingState();

    //when self user send
    QObject::connect(
        this->_rpzClient, &RPZClient::ackIdentity,
        [&](const QVariantHash &userHash) {
            RPZUser rpz_user(userHash);

            //define self pen color
            this->hints()->setPenColor(rpz_user.color());

            //if host
            auto descriptor = rpz_user.role() == RPZUser::Role::Host ? NULL : this->_rpzClient->getConnectedSocketAddress();
            bool is_remote = this->_hints->defineAsRemote(descriptor);

            emit remoteChanged(is_remote);
        }
    );

    //when missing assets
    QObject::connect(
        this->_hints, &ViewMapHint::requestMissingAsset,
        this->_rpzClient, &RPZClient::askForAsset
    );

    //when receiving missing asset
    QObject::connect(
        this->_rpzClient, &RPZClient::assetSucessfullyInserted,
        this->_hints, &ViewMapHint::replaceMissingAssetPlaceholders
    );

    //on map change
    QObject::connect(
        this->_rpzClient, &RPZClient::mapChanged,
        this->_hints, &AtomsStorage::handleAlterationRequest
    );

    //when been asked for map content
    QObject::connect(
        this->_rpzClient, &RPZClient::beenAskedForMapHistory,
        this, &MapView::_sendMapHistory
    );

}

void MapView::onRPZClientDisconnect(RPZClient* cc) {

    //back to default state
    this->_hints->defineAsRemote();
    this->_hints->loadDefaultState();

}

void MapView::_sendMapChanges(QVariantHash &payload) {
    AlterationPayload aPayload(payload);
    if(!aPayload.isNetworkRoutable()) return;

    if(!this->_rpzClient) return;
    this->_rpzClient->sendMapChanges(payload);
}

void MapView::_sendMapHistory() {
    ResetPayload payload(this->_hints->atoms());
    this->_sendMapChanges(payload);
}

/////////////////
/* END NETWORK */
/////////////////

//////////
/* TOOL */
//////////

//mouse click
void MapView::mousePressEvent(QMouseEvent *event) {

    //register last position
    this->_lastPointMousePressing = event->pos();
    this->_lastPointMousePressed = event->pos();
    this->_isMousePressed = true;

    switch(event->button()) {
        case Qt::MouseButton::MiddleButton:
            this->_isMiddleToolLock = !this->_isMiddleToolLock;
            this->_changeTool(MapTools::Actions::Scroll, true);
            break;
        case Qt::MouseButton::RightButton:
            //TODO Contextual menu
            break;
        case Qt::MouseButton::LeftButton:
            this->_toolOnMousePress(this->_getCurrentTool());
            break;
    }

    QGraphicsView::mousePressEvent(event);
}

//on movement
void MapView::mouseMoveEvent(QMouseEvent *event) {

    switch(this->_getCurrentTool()) {
        case MapTools::Actions::Draw:
            if(this->_isMousePressed) this->_drawLineTo(event->pos());
            break;
        case MapTools::Actions::Text:
            if(this->_tempText) this->_hints->centerGraphicsItemToPoint(this->_tempText, event->pos());
            break;
    }

    //register last position
    this->_lastPointMousePressing = event->pos();

    QGraphicsView::mouseMoveEvent(event);
}


//mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {

    switch(event->button()) {
        case Qt::MouseButton::RightButton:
            this->_changeTool(MapTools::Actions::None, true);
            break;
        case Qt::MouseButton::MiddleButton:
            if(!this->_isMiddleToolLock) {
                this->_changeTool(MapTools::Actions::None, true);
            }
            break;
        case Qt::MouseButton::LeftButton:
            this->_toolOnMouseRelease(this->_getCurrentTool());
            break;
    }

    this->_isMousePressed = false;

    QGraphicsView::mouseReleaseEvent(event);
}

void MapView::_toolOnMousePress(const MapTools::Actions &tool) {
    switch(tool) {
        case MapTools::Actions::Draw: {
            this->_beginDrawing();
        }
        break;

        case MapTools::Actions::Text: {
            this->_hints->turnGhostTextIntoDefinitive(this->_tempText, this->_lastPointMousePressed);
        }
        break;
    }
}

void MapView::_toolOnMouseRelease(const MapTools::Actions &tool) {
    
    //if was drawing...
    if(this->_tempDrawing)  {
        this->_endDrawing();
    }

    //if was moving ?
    this->hints()->handleAnyMovedItems();
}


//returns tool
MapTools::Actions MapView::_getCurrentTool() const {
    return this->_quickTool == MapTools::Actions::None ? this->_selectedTool : this->_quickTool;
}

//change tool
void MapView::_changeTool(MapTools::Actions newTool, const bool quickChange) {
    
    this->_clearTempText();
    this->_endDrawing();

    //if quick change asked
    if(quickChange) {

        this->_quickTool = newTool;

        //if unselecting quicktool
        if(newTool == MapTools::Actions::None) {
            newTool = this->_selectedTool;
            this->_generateTempText();
        }   

    } else {

        //define new tool
        this->_selectedTool = newTool;
        this->_scene->clearSelection();

    }    

    //if a quicktool is selected
    if(this->_quickTool != MapTools::Actions::None) {
        newTool = this->_quickTool;
    }
    
    switch(newTool) {
        case MapTools::Actions::Draw:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(Qt::CrossCursor);
            break;
        case MapTools::Actions::Text:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(Qt::IBeamCursor);
            break;
        case MapTools::Actions::Scroll:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
            this->setCursor(Qt::ClosedHandCursor);
            break;
        case MapTools::Actions::Select:
        default:
            this->setInteractive(true);
            this->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
            this->setCursor(Qt::ArrowCursor);
            this->_isMiddleToolLock = false;
    }
}

//on received event
void MapView::changeToolFromAction(const MapTools::Actions &instruction) {

    //oneshot instructions switch
    switch(instruction) {
        case MapTools::Actions::ResetView:
            this->_goToDefaultViewState();
            return;
    }

    //else select the new tool
    return this->_changeTool(instruction);

}

//////////////
/* END TOOL */
//////////////

//////////
/* MOVE */
//////////

void MapView::_goToSceneCenter() {
    auto center = this->_scene->sceneRect().center();
    this->centerOn(center);
}

void MapView::_animatedMove(const Qt::Orientation &orientation, int correction) {
    
    //prepare
    auto bar = orientation == Qt::Orientation::Vertical ? this->verticalScrollBar() : this->horizontalScrollBar();
    auto controller = orientation == Qt::Orientation::Vertical ? AnimationTimeLine::Type::VerticalMove : AnimationTimeLine::Type::HorizontalMove;

    //define animation handler
    AnimationTimeLine::use(controller, correction, this, [bar](qreal base, qreal prc) {
        bar->setValue(bar->value() + base);
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

    };

    AnimationTimeLine::use(AnimationTimeLine::Type::Zoom, zoomRatioToApply, this, zoom);

};


//////////////
/* END ZOOM */
//////////////

//////////
/* DROP */
//////////

void MapView::dropEvent(QDropEvent *event) {
    if(!this->_droppableGraphicsItem) return;

    //definitive appending for temporary graphicsItem
    this->_hints->turnGhostItemIntoDefinitive(
        this->_droppableGraphicsItem,
        this->_droppableElement, 
        event->pos()
    );

    this->_droppableGraphicsItem = nullptr;
}

void MapView::dragMoveEvent(QDragMoveEvent * event) {
    if(!this->_droppableGraphicsItem) return;
    event->acceptProposedAction();

    //update dummy graphics item
    this->_hints->centerGraphicsItemToPoint(this->_droppableGraphicsItem, event->pos());
}

void MapView::dragEnterEvent(QDragEnterEvent *event) {

    //reset previous temp values
    if(this->_droppableGraphicsItem) delete this->_droppableGraphicsItem;
    this->_droppableGraphicsItem = nullptr;
    this->_droppableElement = nullptr;
    this->_droppableSourceDatabase = nullptr;

    //if has a widget attached, OK
    if(event->source()) {
        
        //check selected items in source
        auto droppableSource = (AssetsTreeView*)event->source();
        auto selectedIndexes = droppableSource->selectedElementsIndexes();
        if(selectedIndexes.count() != 1) return;

        //check if item
        auto selectedElem = AssetsDatabaseElement::fromIndex(selectedIndexes.takeFirst());
        if(!selectedElem->isItem()) return;

        //type restriction
        if(selectedElem->type() != AssetsDatabaseElement::Type::Object) return; 

        //update temporary values
        this->_droppableElement = selectedElem;
        this->_droppableGraphicsItem = this->_hints->generateGhostItem(this->_droppableElement);

        //accept
        event->acceptProposedAction();
    }

}

void MapView::dragLeaveEvent(QDragLeaveEvent *event) {
    //reset previous temp values
    if(this->_droppableGraphicsItem) delete this->_droppableGraphicsItem;
    this->_droppableGraphicsItem = nullptr;
    this->_droppableElement = nullptr;
    this->_droppableSourceDatabase = nullptr;
}

//////////////
/* END DROP */
//////////////

/////////////
/* DRAWING */
/////////////

void MapView::_beginDrawing() {
    
    //destroy temp
    if(this->_tempDrawing) {
        delete this->_tempDrawing;
        this->_tempDrawing = nullptr;
    }

    auto startPoint = this->mapToScene(this->_lastPointMousePressed);
    this->_tempDrawing = this->scene()->addPath(QPainterPath(QPointF(0,0)), this->hints()->getPen());
    this->_tempDrawing->setPos(startPoint);
}

void MapView::_endDrawing() {
    if(!this->_tempDrawing) return;
    if(this->_tempDrawing->path().elementCount() < 2) return;

    //add definitive path
    this->_hints->addDrawing(
        this->_tempDrawing->scenePos(), 
        this->_tempDrawing->path(), 
        this->hints()->getPen()
    );

    //destroy temp
    delete this->_tempDrawing;
    this->_tempDrawing = nullptr;
}

void MapView::_drawLineTo(const QPoint &evtPoint) {

    auto existingPath = this->_tempDrawing->path();

    //define vector
    auto to = this->mapToScene(evtPoint);
    auto to2 = this->_tempDrawing->mapFromScene(to);

    //save as new path
    existingPath.lineTo(to2);
    this->_tempDrawing->setPath(existingPath);
}

/////////////////
/* END DRAWING */
/////////////////
