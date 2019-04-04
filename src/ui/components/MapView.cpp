#include "MapView.h"


MapView::MapView(QWidget *parent) : QGraphicsView(parent), _scene(new QGraphicsScene) {
    
    //default
    this->setAcceptDrops(true);
    this->setInteractive(true);

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

void MapView::toolSelectionChanged(QAction *action) {
    
    //go by default tool if unchecked
    auto state = action->isChecked();
    if(!state) return this->_changeTool(MapView::_defaultTool);

    //else select the new tool
    auto tool = (MapTools::Actions)action->data().toInt();
    return this->_changeTool(tool);
}


//mouse click
void MapView::mousePressEvent(QMouseEvent *event) {

    switch(event->button()) {
        case Qt::MouseButton::MiddleButton:
            this->_changeTool(MapTools::Actions::Scroll, true);
            break;
        case Qt::MouseButton::RightButton:
            this->_changeTool(MapTools::Actions::Rotate, true);
            break;
        case Qt::MouseButton::LeftButton:
            if(this->_selectedTool == MapTools::Actions::Draw) {
                this->_tempDrawing = new QPainterPath(this->mapToScene(event->pos()));
            }
            break;
    }

    //register last position
    this->_lastPointMousePressed = event->pos();
    this->_lastPointMouseClick = event->pos();
    this->_isMousePressed = true;
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
        case MapTools::Actions::Scroll:
            this->_scroll(event->pos());
            break;
    }

    //register last position
    this->_lastPointMousePressed = event->pos();
}


//mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {

    switch(event->button()) {
        case Qt::MouseButton::RightButton:
        case Qt::MouseButton::MiddleButton:
            this->_changeTool(MapTools::Actions::None, true);
            break;
        case Qt::MouseButton::LeftButton:
            if(this->_selectedTool == MapTools::Actions::Draw) {
                
                //add definitive path
                auto newPath = this->_scene->addPath(*this->_tempDrawing, this->_getPen());
                this->_selfDrawings.insert(QUuid::createUuid(), newPath);
                this->_tempDrawing = nullptr;
                
                //destroy temp
                auto iGroup = this->_scene->createItemGroup(this->_tempLines);
                this->_scene->destroyItemGroup(iGroup);
                this->_tempLines.clear();

            }
            break;
    }

    this->_isMousePressed = false;
}

//tool
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
            this->setCursor(Qt::CrossCursor);
            break;
        case MapTools::Actions::Scroll:
            this->setCursor(Qt::ClosedHandCursor);
            break;
        case MapTools::Actions::Rotate:
            this->setCursor(*this->_rotateCursor);
            break;
        default:
            this->setCursor(Qt::ArrowCursor);
    }
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
    auto way = this->_lastPointMousePressed - evtPoint;
    auto pp = ((double)way.y()) / 5;
    this->rotate(pp);
}

////////////////
/* END ROTATE */
////////////////

////////////
/* SCROLL */
////////////

void MapView::_scroll(const QPoint &evtPoint) {

    QScrollBar *hBar = this->horizontalScrollBar();
    QScrollBar *vBar = this->verticalScrollBar();
    QPoint delta = evtPoint - this->_lastPointMousePressed;

    auto h = hBar->value() + (isRightToLeft() ? delta.x() : -delta.x());
    auto v = vBar->value() - delta.y();
    hBar->setValue(h);
    vBar->setValue(v);
}

////////////////
/* END SCROLL */
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

void MapView::_drawLineTo(const QPoint &evtPoint) {

    //save
    this->_tempDrawing->lineTo(this->mapToScene(evtPoint));

    //draw temp line
    auto lineCoord = QLineF(this->mapToScene(this->_lastPointMousePressed), this->mapToScene(evtPoint));
    auto tempLine = this->_scene->addLine(lineCoord, this->_getPen());
    this->_tempLines.append(tempLine);
}

QPen MapView::_getPen() {
    return QPen(this->_penColor, this->_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}
