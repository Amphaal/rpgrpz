#pragma once 

#include "src/ui/map/MapView.h"

#include <QPen>

class MiniMapView : public QGraphicsView {
    public:
        MiniMapView(MapView* master, QWidget *parent = nullptr) : QGraphicsView(master->scene(), parent), _master(master) {

            this->setVisible(false);
            this->_visibleAsap = AppContext::settings()->minimapActive();

            //events
            this->_handleHintsSignalsAndSlots();

            //OpenGL backend activation
            QGLFormat format;
            format.setSampleBuffers(true);
            format.setDirectRendering(true);
            format.setAlpha(true);
            this->setViewport(new QGLWidget(format));

            this->setRenderHints(QPainter::Antialiasing);

            //hide scrollbars
            this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
                this->_fitMapInMiniMap();
            }
        }

    private:
        MapView* _master = nullptr;
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

        void _fitMapInMiniMap() {
            auto size = this->_getMinimumSizeSceneRect();
            this->fitInView(size, Qt::AspectRatioMode::KeepAspectRatio);
        }

        void _handleHintsSignalsAndSlots() {
            
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

                    this->_fitMapInMiniMap();

                }
            );

            //
            QObject::connect(
                this->_master, &MapView::cameraMoved,
                [=]() {
                    // this->repaint();
                }
            );

        }

    protected:
        void wheelEvent(QWheelEvent * event) override {};
        void keyPressEvent(QKeyEvent *event) override {};
        void mousePressEvent(QMouseEvent *event) override {};

        void drawForeground(QPainter *painter, const QRectF &rect) override {
            
            // painter->save();

            // QPen pen;
            // pen.setWidth(1);
            // pen.setCosmetic(true);

            // auto rectA = this->_master->mapToScene(this->viewport()->rect()).boundingRect();

            // painter->setPen(pen);
            // painter->drawRect(rectA);

            // painter->restore();

        }

        void paintEvent(QPaintEvent *event) override {
            qDebug() << "reprint";
            QGraphicsView::paintEvent(event);
        }

};