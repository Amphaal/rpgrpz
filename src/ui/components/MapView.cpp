#include "MapView.h"


MapView::MapView(QWidget *parent) : QGraphicsView(parent), _scene(new QGraphicsScene), _latestPosDrop(new QPoint) {
    
    this->setAcceptDrops(true);
    this->setInteractive(true);
    
    //openGL activation
    this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DirectRendering)));
    this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->update();

    //background
    auto background = new QBrush("#EEE", Qt::CrossPattern);
    this->setBackgroundBrush(*background);
    this->setCacheMode(QGraphicsView::CacheBackground);

    //optimisations
    //this->setOptimizationFlags( QFlags<OptimizationFlag>(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing));

    //define scene
    this->setScene(this->_scene);
}

void MapView::toolSelectionChanged(QAction *action) {
    auto tool = (MapTools::Actions)action->data().toInt();
    auto state = action->isChecked();

    switch(tool) {
        case MapTools::Actions::Draw:
            
            break;
    }
}

//////////
/* TOOL */
//////////

void MapView::mouseMoveEvent(QMouseEvent *event) {
    switch(this->_selectedTool) {
        case MapTools::Actions::Draw:
                this->_drawLineTo(this->mapToScene(event->pos()));
            break;
    }
}

void MapView::mousePressEvent(QMouseEvent *event) {
    
    if (event->button() == Qt::MouseButton::MiddleButton) {
        this->_selectedTool = MapTools::Actions::Navigate;
        this->setCursor(Qt::SizeAllCursor);
    }

    this->_lastPoint = this->mapToScene(event->pos());
}

void MapView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MouseButton::MiddleButton) {
        this->_selectedTool = MapTools::Actions::Draw;
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
    auto factor = qPow(1.2, event->delta() / 240.0);
    this->_zoomBy(factor);
};

qreal MapView::zoomFactor() const
{
    return this->transform().m11();
}

void MapView::_zoomBy(qreal factor)
{
    const qreal currentZoom = this->zoomFactor();
    this->scale(factor, factor);
}

//////////////
/* END ZOOM */
//////////////

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
    this->_lastPoint = endPoint;
}