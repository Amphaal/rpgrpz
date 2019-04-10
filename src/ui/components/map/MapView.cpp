#include "MapView.h"

MapView::MapView(QWidget *parent) : QGraphicsView(parent) {

    //default
    this->setScene(
        new QGraphicsScene(
            this->_defaultSceneSize,  
            this->_defaultSceneSize,  
            this->_defaultSceneSize, 
            this->_defaultSceneSize, 
            this
        )
    );
    this->_hints = new MapHintViewBinder(this); //after first inst of scene
    this->setAcceptDrops(true);
    this->_changeTool(MapView::_defaultTool);

    //custom cursors
    this->_rotateCursor = new QCursor(QPixmap(":/icons/app/tools/rotate.png"));
    
    //openGL activation
    this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DirectRendering)));
    this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->update();

    //background
    auto background = new QBrush("#EEE", Qt::CrossPattern);
    this->setBackgroundBrush(*background);
    //this->setCacheMode(QGraphicsView::CacheBackground);

    //optimisations
    //this->setOptimizationFlags( QFlags<OptimizationFlag>(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing));

    //to route from MapHints
    QObject::connect(
        this->_hints, &MapHint::assetsAlteredForLocal,
        [&](const RPZAsset::Alteration &state, QVector<RPZAsset> &elements) {
            emit assetsAlteredForLocal(state, elements);
        }
    );
    
    //to route from MapHints
    QObject::connect(
        this->_hints, &MapHint::assetsAlteredForNetwork,
        this, &MapView::_sendMapChanges
    );


    //default state
    this->scale(this->_defaultScale, this->_defaultScale);
    this->_goToDefaultViewState();
    
}

void MapView::_goToDefaultViewState() {
    this->_goToSceneCenter();
    this->_goToDefaultZoom();
    this->_rotateBackToNorth();
}

void MapView::_goToSceneCenter() {
    auto center = this->scene()->sceneRect().center();
    qDebug() << "centering on " << center;
    this->centerOn(center);
}


MapHintViewBinder* MapView::hints() {
    return this->_hints;
}

void MapView::keyPressEvent(QKeyEvent * event) {
    
    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->_hints->alterSceneFromItems(RPZAsset::Alteration::Removed, this->scene()->selectedItems());
            break;
        
        //ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->_changeTool(MapView::_defaultTool);
            emit unselectCurrentToolAsked();
            break;
    }

}

/////////////
/* NETWORK */
/////////////


void MapView::bindToRPZClient(RPZClient * cc) {

    ClientBindable::bindToRPZClient(cc);

    //on map change
    QObject::connect(
        this->_rpzClient, &RPZClient::mapChanged,
        this->_hints, &MapHintViewBinder::unpackFromNetworkReceived
    );
    
    //when self user send
    QObject::connect(
        this->_rpzClient, &RPZClient::ackIdentity,
        [&](const QVariantHash &user) {
            this->_penColor = RPZUser::fromVariantHash(user).color();
        }
    );

    //when been asked for map content
    QObject::connect(
        this->_rpzClient, &RPZClient::beenAskedForMapHistory,
        this, &MapView::_sendMapHistory
    );

}

void MapView::_sendMapChanges(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements) {
    if(!this->_rpzClient) return;

    auto data = this->_hints->packageForNetworkSend(state, elements);

    this->_rpzClient->sendMapChanges(data, false);
}

void MapView::_sendMapHistory() {
    if(!this->_rpzClient) return;

    auto data = this->_hints->packageForNetworkSend(RPZAsset::Alteration::Reset, this->_hints->fetchHistory());

    this->_rpzClient->sendMapChanges(data, true);
}

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
            this->_changeTool(MapTools::Actions::Rotate, true);
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
            this->_drawLineTo(event->pos());
            break;
        case MapTools::Actions::Rotate:
            this->_rotate(event->pos());
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
    }
}

void MapView::_toolOnMouseRelease(const MapTools::Actions &tool) {
    if(this->_tempDrawing)  {
        this->_endDrawing();
    }
}


//returns tool
MapTools::Actions MapView::_getCurrentTool() const {
    return this->_quickTool == MapTools::Actions::None ? this->_selectedTool : this->_quickTool;
}

//change tool
void MapView::_changeTool(MapTools::Actions newTool, const bool quickChange) {

    if(quickChange) {
        this->_quickTool = newTool;
        if(newTool == MapTools::Actions::None) {
            newTool = this->_selectedTool;
        }   
    } else {
        this->_selectedTool = newTool;
        this->scene()->clearSelection();
    }    
    
    switch(newTool) {
        case MapTools::Actions::Draw:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(Qt::CrossCursor);
            break;
        case MapTools::Actions::Rotate:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(*this->_rotateCursor);
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
    }
}

//on received event
void MapView::changeToolFromAction(QAction *action) {
    
    //cast inner data to enum
    const auto instruction = (MapTools::Actions)action->data().toInt();
    
    //oneshot instructions switch
    switch(instruction) {
        case MapTools::Actions::RotateToNorth:
            this->_rotateBackToNorth();
            return;
        case MapTools::Actions::ResetView:
            this->_goToDefaultViewState();
            return;
    }

    //go by default tool if unchecked
    auto state = action->isChecked();
    if(!state) return this->_changeTool(MapView::_defaultTool);

    //else select the new tool
    return this->_changeTool(instruction);

}

void MapView::changePenSize(const int newSize) {
    this->_penWidth = newSize;
}

//////////////
/* END TOOL */
//////////////

//////////
/* ZOOM */
//////////

void MapView::wheelEvent(QWheelEvent *event) {

    //make sure no button is pressed
    if(this->_isMousePressed) return;

    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15; // see QWheelEvent documentation
    this->_numScheduledScalings += numSteps;
    
    if (this->_numScheduledScalings * numSteps < 0) {
        this->_numScheduledScalings = numSteps; // if user moved the wheel in another direction, we reset previously scheduled scalings
    } 
    
    QTimeLine *anim = new QTimeLine(350, this);
    anim->setUpdateInterval(20);

    QObject::connect(anim, &QTimeLine::valueChanged, this, &MapView::_zoomBy_scalingTime);
    QObject::connect(anim, &QTimeLine::finished, this, &MapView::_zoomBy_animFinished);
    
    anim->start();
};

void MapView::_zoomBy_scalingTime(qreal x) {
    qreal factor = 1.0 + qreal(this->_numScheduledScalings) / 300.0;
    this->_currentRelScale = factor * this->_currentRelScale;
    this->scale(factor, factor);
    //qDebug() << "zooming by " << factor << " (" << qreal(this->_currentRelScale) << " from initial)";
}

void MapView::_goToDefaultZoom() {
    auto corrected = 1/(this->_currentRelScale);
    this->scale(corrected, corrected);
    this->_currentRelScale = 1;
}

void MapView::_zoomBy_animFinished() {
    if (this->_numScheduledScalings > 0) {
        this->_numScheduledScalings--;
    } else {
        this->_numScheduledScalings++;
        this->sender()->deleteLater();
    }
}

//////////////
/* END ZOOM */
//////////////

////////////
/* ROTATE */
////////////

void MapView::_rotate(const QPoint &evtPoint) {
    const auto way = this->_lastPointMousePressing - evtPoint;
    auto pp = ((double)way.y()) / 5;
    this->_degreesFromNorth += pp; 
    this->rotate(pp);

    qDebug() << "rotating " << qreal(pp) << " deg (" << qreal(this->_degreesFromNorth) << " from north)";
}

void MapView::_rotateBackToNorth() {
    
    if(this->_degreesFromNorth == 0) return;

    auto adjust = fmod(this->_degreesFromNorth, 360);
    this->rotate(-this->_degreesFromNorth);
    
    qDebug() << "rotating " << qreal(-this->_degreesFromNorth) << "deg back to north)";
    
    this->_degreesFromNorth = 0;
}

////////////////
/* END ROTATE */
////////////////

//////////
/* DROP */
//////////

void MapView::dropEvent(QDropEvent *event) {
    
    // auto source = (AssetsNavigator*)event->source();
    // auto si = source->selectedItems();
    // auto toRepresent = si[0]->data(1, Qt::UserRole);
    

    // QPointF point = this->mapToScene(event->pos());
    // item->setPos(point);
    // auto item = new QGraphicsSvgItem("C:/Users/Amphaal/Desktop/pp.svg");
    // this->scene()->addItem(item);

    // //update latest for auto inserts
    // this->_latestPosDrop->setX(this->_latestPosDrop->x() + 10);
    // this->_latestPosDrop->setY(this->_latestPosDrop->y() + 10);
}

void MapView::dragMoveEvent(QDragMoveEvent * event) {
    event->acceptProposedAction();
}

void MapView::dragEnterEvent(QDragEnterEvent *event) {
    
    //if has a widget attached, OK
    if(event->source()) {
        event->acceptProposedAction();
    }

}

//////////////
/* END DROP */
//////////////

/////////////
/* DRAWING */
/////////////

void MapView::_beginDrawing() {
    this->_tempDrawing = new QPainterPath(this->mapToScene(this->_lastPointMousePressed));
}

void MapView::_endDrawing() {
    
    //if something has been drawn
    if(this->_tempLines.size()) {

        //add definitive path
        auto currPen = this->_getPen();
        auto drawing = this->_hints->addDrawing(*this->_tempDrawing, currPen);
        auto metadata = QVariantHash();
        metadata["w"] = currPen.width();
        auto newAsset = RPZAsset(AssetBase::Type::Drawing, drawing, metadata);
        this->_hints->alterSceneFromAsset(RPZAsset::Alteration::Added, newAsset);
    }

    this->_tempDrawing = nullptr;

    //destroy temp
    for(auto &i : this->_tempLines) {
        delete i;
    }
    this->_tempLines.clear();
}

void MapView::_drawLineTo(const QPoint &evtPoint) {

    //save
    this->_tempDrawing->lineTo(this->mapToScene(evtPoint));

    //draw temp line
    const auto lineCoord = QLineF(this->mapToScene(this->_lastPointMousePressing), this->mapToScene(evtPoint));
    auto tempLine = this->scene()->addLine(lineCoord, this->_getPen());
    this->_tempLines.append(tempLine);
}

QPen MapView::_getPen() const {
    return QPen(this->_penColor, this->_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

/////////////////
/* END DRAWING */
/////////////////
