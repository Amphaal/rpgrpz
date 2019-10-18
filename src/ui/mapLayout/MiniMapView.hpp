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
                [=]() {
                    this->endHeavyLoadPlaceholder();
                    auto i = this->_getMinimumSizeSceneRect();
                    this->fitInView(i);
                }
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

        }

    private:
        QGraphicsView* _view = nullptr;

        QRectF _getMinimumSizeSceneRect() {
            QRectF minimalSize(QPointF(0, 0), QSizeF(640, 640));
            minimalSize.moveCenter(this->scene()->sceneRect().center());

            auto itemsBoundingRect = this->scene()->itemsBoundingRect();
            auto rectSize = itemsBoundingRect.size();

            //TODO itemsBoundingRect claqué ?

            if(rectSize.width() < minimalSize.width() || rectSize.height() < minimalSize.height()) {
                return minimalSize;
            }

            return itemsBoundingRect;
            
        }

    protected:
        void wheelEvent(QWheelEvent * event) override {};
        void keyPressEvent(QKeyEvent *event) override {};
        void mousePressEvent(QMouseEvent *event) override {};

        void drawForeground(QPainter *painter, const QRectF &rect) override {
            this->mayUpdateHeavyLoadPlaceholder(painter);
        }

        void drawBackground(QPainter *painter, const QRectF &rect) override {
            this->drawBackgroundCheckerboard(painter, rect);
        }
};