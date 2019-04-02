#include "MapView.h"


MapView::MapView(QWidget *parent) : QGraphicsView(parent), _scene(new QGraphicsScene), _latestPosDrop(new QPoint) {
    
    this->setMinimumSize(400,400);
    this->setAcceptDrops(true);
    this->setInteractive(true);
    this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    auto background = new QBrush("#EEE", Qt::CrossPattern);
    this->setBackgroundBrush(*background);

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
/* MOVE */
//////////

void MapView::mouseMoveEvent(QMouseEvent *event) {

}

void MapView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MouseButton::MiddleButton) {
        this->_moveMode = true;
        this->setCursor(Qt::SizeAllCursor);
    }
}

void MapView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MouseButton::MiddleButton) {
        this->_moveMode = false;
        this->setCursor(Qt::ArrowCursor);
    }
}


//////////////
/* END MOVE */
//////////////

//////////
/* ZOOM */
//////////

void MapView::wheelEvent(QWheelEvent *event) {
    auto factor = qPow(1.2, event->delta() / 240.0);
    this->zoomBy(factor);
};

qreal MapView::zoomFactor() const
{
    return this->transform().m11();
}

void MapView::zoomBy(qreal factor)
{
    const qreal currentZoom = this->zoomFactor();
    this->scale(factor, factor);
}

//////////////
/* END ZOOM */
//////////////


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