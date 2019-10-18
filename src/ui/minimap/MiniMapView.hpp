#pragma once 

#include <QGraphicsView>
#include <QGLWidget>

class MiniMapView : public QGraphicsView {
    public:
        MiniMapView(QGraphicsScene* scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) {
            
            //OpenGL backend activation
            QGLFormat format;
            format.setSampleBuffers(true);
            format.setDirectRendering(true);
            format.setAlpha(true);
            this->setViewport(new QGLWidget(format));

            this->setDragMode(QGraphicsView::NoDrag);

            //force viewport update mode
            this->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::SmartViewportUpdate); 

        }
};