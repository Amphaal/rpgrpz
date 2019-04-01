#include "MapView.h"


MapView::MapView(QWidget *parent) : QGraphicsView(parent), _scene(new QGraphicsScene) {
    
    this->setMinimumSize(400,400);

    auto background = new QBrush("#EEE", Qt::CrossPattern);

    this->setBackgroundBrush(*background);

    this->setScene(this->_scene);
    
    //this->_scene->addItem(new QGraphicsSvgItem("C:/Users/Amphaal/Desktop/pp.svg"));

}

void MapView::wheelEvent(QWheelEvent *event) {
    this->zoomBy(qPow(1.2, event->delta() / 240.0));
};

qreal MapView::zoomFactor() const
{
    return this->transform().m11();
}

void MapView::zoomBy(qreal factor)
{
    const qreal currentZoom = this->zoomFactor();
    if ((factor < 1 && currentZoom < 0.1) || (factor > 1 && currentZoom > 10))
        return;
    this->scale(factor, factor);
}