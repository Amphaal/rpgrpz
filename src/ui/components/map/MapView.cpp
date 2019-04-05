#include "MapView.h"


MapView::MapView(QWidget *parent) : QGraphicsView(parent), _scene(new QGraphicsScene) {
    
    //default
    this->setAcceptDrops(true);
    this->_changeTool(MapView::_defaultTool);

    //custom cursors
    this->_rotateCursor = new QCursor(QPixmap(":/icons/app/rotate.png"));
    
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

    //define scene
    this->_scene->setSceneRect(35000, 35000, 35000, 35000);
    this->setScene(this->_scene);
}

///////////////
/* FRAMERATE */
///////////////

void MapView::_instFramerate() {
    auto timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, &MapView::_framerate_oneSecondTimeout);
    timer->setInterval(1000);
    timer->start();
}

void MapView::_framerate_oneSecondTimeout() {
    this->_frameRate=(this->_frameRate + this->_numFrames)/2;
    qInfo() << this->_frameRate;
    this->_numFrames=0;
}

///////////////////
/* END FRAMERATE */
///////////////////

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
            this->_toolOnMousePress(this->_selectedTool);
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
            if(this->_isMiddleToolLock) {
                this->_changeTool(MapTools::Actions::None, true);
            }
            break;
        case Qt::MouseButton::LeftButton:
            this->_toolOnMouseRelease(this->_selectedTool);
            break;
    }

    this->_isMousePressed = false;

    QGraphicsView::mouseReleaseEvent(event);
}

void MapView::_toolOnMousePress(MapTools::Actions tool) {
    switch(tool) {
        case MapTools::Actions::Draw: {
            this->_beginDrawing();
        }
        break;
    }
}

void MapView::_toolOnMouseRelease(MapTools::Actions tool) {
    switch(tool) {
        case MapTools::Actions::Draw: {
            this->_endDrawing();
        }
        break;
    }
}


//returns tool
MapTools::Actions MapView::_getCurrentTool() {
    return this->_quickTool == MapTools::Actions::None ? this->_selectedTool : this->_quickTool;
}

//change tool
void MapView::_changeTool(MapTools::Actions newTool,  bool quickChange) {
    
    if(quickChange) {
        this->_quickTool = newTool;
        if(newTool == MapTools::Actions::None) {
            newTool = this->_selectedTool;
        }   
    } else {
        this->_selectedTool = newTool;
    }    
    
    switch(newTool) {
        case MapTools::Actions::Draw:
            this->setInteractive(true);
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
    
    //go by default tool if unchecked
    auto state = action->isChecked();
    if(!state) return this->_changeTool(MapView::_defaultTool);

    //else select the new tool
    auto tool = (MapTools::Actions)action->data().toInt();
    return this->_changeTool(tool);

}

void MapView::changePenSize(int newSize) {
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
    qreal factor = 1.0+ qreal(this->_numScheduledScalings) / 300.0;
    this->scale(factor, factor);
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
    auto way = this->_lastPointMousePressing - evtPoint;
    auto pp = ((double)way.y()) / 5;
    this->rotate(pp);
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
    // this->_scene->addItem(item);

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
    //add definitive path
    auto newPath = this->_scene->addPath(*this->_tempDrawing, this->_getPen());
    newPath->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    ));
    this->_alterElement(MapElementEvtState::Added, newPath);
    this->_tempDrawing = nullptr;
    
    //destroy temp
    auto iGroup = this->_scene->createItemGroup(this->_tempLines);
    for(auto i : this->_tempLines) {
        delete i;
    }
    this->_scene->destroyItemGroup(iGroup);
    this->_tempLines.clear();
}

void MapView::_drawLineTo(const QPoint &evtPoint) {

    //save
    this->_tempDrawing->lineTo(this->mapToScene(evtPoint));

    //draw temp line
    auto lineCoord = QLineF(this->mapToScene(this->_lastPointMousePressing), this->mapToScene(evtPoint));
    auto tempLine = this->_scene->addLine(lineCoord, this->_getPen());
    this->_tempLines.append(tempLine);
}

QPen MapView::_getPen() {
    return QPen(this->_penColor, this->_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

/////////////////
/* END DRAWING */
/////////////////

//////////////
/* ELEMENTS */
//////////////

//register actions
QUuid MapView::_alterElementInternal(MapElementEvtState alteration, QGraphicsItem* element, JSONSocket* owner) {
    
    //temp
    QUuid thisElemUuid;
    switch(alteration) {
        
        //on addition
        case MapElementEvtState::Added:
            thisElemUuid = QUuid::createUuid();
            if(owner) {
                if(!this->_elementsByOwner.contains(owner)) {
                    this->_elementsByOwner.insert(owner, QSet<QUuid>());
                }
                this->_elementsByOwner[owner].insert(thisElemUuid);
            } else {
                this->_selfElements.insert(thisElemUuid);
            }
            this->_elementById.insert(thisElemUuid, element);
            this->_IdByElement.insert(element, thisElemUuid);
            break;

        //on removal
        case MapElementEvtState::Removed:
            thisElemUuid = this->_IdByElement[element];
           if(owner) {
               this->_elementsByOwner[owner].remove(thisElemUuid);
            } else {
                this->_selfElements.remove(thisElemUuid);
            }
            this->_elementById.remove(thisElemUuid);
            this->_IdByElement.remove(element);
            break;
    }

    return thisElemUuid;
}

void MapView::_alterElement(MapElementEvtState alteration, QGraphicsItem* element, JSONSocket* owner) {
    QList<QGraphicsItem*> list;
    list.append(element);
    return this->_alterElements(alteration, list, owner);
}

void MapView::_alterElements(MapElementEvtState alteration, QList<QGraphicsItem*> elements, JSONSocket* owner) {
    
    QHash<QUuid, QGraphicsItem*> mapToEvt;

    for(auto elem : elements) {
        auto uuid = this->_alterElementInternal(alteration, elem, owner);
        mapToEvt.insert(uuid, elem);
    }

    //emit event
    emit mapElementsAltered(mapToEvt, alteration);
}

void MapView::keyPressEvent(QKeyEvent * event) {
    
    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            if(this->_scene->selectedItems().length()) {
                this->_alterElements(MapElementEvtState::Removed, this->_scene->selectedItems());
                for(auto i : this->_scene->selectedItems()) {
                    delete i;
                }
            }
            break;
        
        //ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->_changeTool(MapView::_defaultTool);
            emit unselectCurrentToolAsked();
            break;
    }

}

//////////////////
/* END ELEMENTS */
//////////////////
