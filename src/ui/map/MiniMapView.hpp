#pragma once 

#include <QGraphicsView>
#include <QGLWidget>
#include <QGraphicsOpacityEffect>


#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/ui/map/modules/MV_HUDLayout.hpp"


class MiniMapView : public QGraphicsView {
    public:
        MiniMapView(QGraphicsScene* sceneToMimic, QWidget *parent = nullptr) : QGraphicsView(sceneToMimic, parent) {

            this->setVisible(false);
            this->_visibleAsap = AppContext::settings()->minimapActive();

            //on map loading, set/unset placeholder...
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
                [=]() {
                    this->_alterationOngoing = true;
                    this->setVisible(false);
                }
            );
            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessed,
                [=]() {

                    this->_alterationOngoing = false;
                    this->setVisible(this->_visibleAsap);

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

            this->setCacheMode(QFlags<QGraphicsView::CacheModeFlag>(QGraphicsItem::CacheMode::ItemCoordinateCache));

            //force viewport update mode
            this->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::SmartViewportUpdate);
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum); 

            this->setDragMode(QGraphicsView::NoDrag);
            this->setFixedSize(QSize(240, 240));

        }

        void setAsapVisibility(bool visible) {
            this->_visibleAsap = visible;
            if(!this->_alterationOngoing) {
                this->setVisible(this->_visibleAsap);
            }
        }

    private:
        bool _visibleAsap = false;
        bool _alterationOngoing = false;

        QRectF _getMinimumSizeSceneRect() {
            QRectF minimalSize(QPointF(0, 0), QSizeF(640, 640));
            minimalSize.moveCenter(this->scene()->sceneRect().center());

            auto itemsBoundingRect = this->scene()->itemsBoundingRect();
            auto rectSize = itemsBoundingRect.size();

            if(rectSize.width() < minimalSize.width() || rectSize.height() < minimalSize.height()) {
                return minimalSize;
            }

            return itemsBoundingRect;
            
        }

    protected:
        void wheelEvent(QWheelEvent * event) override {};
        void keyPressEvent(QKeyEvent *event) override {};
        void mousePressEvent(QMouseEvent *event) override {};

};