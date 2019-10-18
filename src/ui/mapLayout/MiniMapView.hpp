#pragma once 

#include <QGraphicsView>
#include <QGLWidget>

#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/ui/map/modules/MV_HUDLayout.hpp"

class MiniMapView : public QGraphicsView, public MV_HUDLayout {
    public:
        MiniMapView(QGraphicsView* viewToMimic, QWidget *parent = nullptr) : QGraphicsView(viewToMimic->scene(), parent), MV_HUDLayout(this), 
            _view(viewToMimic) {
            
            //on map loading, set/unset placeholder...
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
                [=]() {this->displayHeavyLoadPlaceholder();}
            );
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessed,
                [=]() {this->endHeavyLoadPlaceholder();}
            );

            //OpenGL backend activation
            QGLFormat format;
            format.setSampleBuffers(true);
            format.setDirectRendering(true);
            format.setAlpha(true);
            this->setViewport(new QGLWidget(format));

            this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

            //hide scrollbars
            this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            //force viewport update mode
            this->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::SmartViewportUpdate);
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum); 

            this->setDragMode(QGraphicsView::NoDrag);
            this->setFixedSize(QSize(240, 240));

            this->fitInView(this->scene()->sceneRect());

        }

    private:
        QGraphicsView* _view = nullptr;

    protected:
        void wheelEvent(QWheelEvent * event) override {};
        void keyPressEvent(QKeyEvent *event) override {};
        void mousePressEvent(QMouseEvent *event) override {};

        void drawForeground(QPainter *painter, const QRectF &rect) {
            this->mayUpdateHeavyLoadPlaceholder(painter);
        }

        void drawBackground(QPainter *painter, const QRectF &rect) {
            this->drawBackgroundCheckerboard(painter, rect);
        }
};