#include "MapView.h"


MapView::MapView(QWidget *parent) : QGraphicsView(parent), _scene(new QGraphicsScene), _latestPosDrop(new QPoint) {
    
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

//on movement
void MapView::mouseMoveEvent(QMouseEvent *event) {

    //pos relative to scrollarea
    auto relPos = this->mapToScene(event->pos());

    switch(this->_getCurrentTool()) {
        case MapTools::Actions::Draw:
            this->_drawLineTo(relPos);
            
            //register last position
            this->_lastPoint = relPos;
            break;
        case MapTools::Actions::Rotate:
            this->_rotate(relPos);
            break;
    }
}

//mouse click
void MapView::mousePressEvent(QMouseEvent *event) {
    
    switch(event->button()) {
        case Qt::MouseButton::MiddleButton:
            this->_changeTool(MapTools::Actions::Navigate, true);
            break;
        case Qt::MouseButton::RightButton:
            this->_changeTool(MapTools::Actions::Rotate, true);
            break;
    }

    //register last position
    this->_lastPoint = this->mapToScene(event->pos());
}

//mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {

    switch(event->button()) {
        case Qt::MouseButton::RightButton:
        case Qt::MouseButton::MiddleButton:
            this->_changeTool(MapTools::Actions::None, true);
            break;
    }

}

MapTools::Actions MapView::_getCurrentTool() {
    return this->_quickTool == MapTools::Actions::None ? this->_selectedTool : this->_quickTool;
}

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
        case MapTools::Actions::Navigate:
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

void MapView::_rotate(const QPointF &endPoint) {
    auto way = this->_lastPoint.y() - endPoint.y();
    qDebug() << way;
    auto factor =  way < 0 ? .5 : -.5;
    this->rotate(factor);
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

void MapView::_drawLineTo(const QPointF &endPoint)
{

    auto line = QLineF(this->_lastPoint, endPoint);
    auto pen = QPen(this->_penColor, this->_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    this->_scene->addLine(line, pen);

    // int rad = (this->_penWidth / 2) + 2;
    // auto rect = QRect(this->_lastPoint, endPoint);
    // this->_scene->update(rect.normalized().adjusted(-rad, -rad, +rad, +rad));

}