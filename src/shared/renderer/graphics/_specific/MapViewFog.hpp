#pragma once

#include <QObject>
#include <QGraphicsItem>
#include <QPropertyAnimation>
#include <QGraphicsView>
#include <QComboBox>
#include <QPainterPath>

#include "src/shared/models/RPZFogParams.hpp"

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"
#include "src/shared/renderer/graphics/_base/RPZAnimated.hpp"

#include "src/helpers/_appContext.h"

class MapViewFog : public QObject, public QGraphicsItem, public RPZGraphicsItem, public RPZAnimated {
    
    Q_OBJECT
    Q_PROPERTY(qreal textureHPos READ textureHPos WRITE setTextureHPos)
    Q_INTERFACES(QGraphicsItem)
    
    public:
        MapViewFog(const RPZFogParams &params) {
            
            //init from params
            this->_rawPath = params.path();
            this->setFogMode(params.mode(), false);

            this->setZValue(AppContext::FOG_Z_INDEX);

            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);  

            QPixmap texture(":/assets/smoke.png");
            this->_brush.setTexture(texture);

            this->_fogAnim = new QPropertyAnimation(this, "textureHPos", this);
            this->_fogAnim->setEasingCurve(QEasingCurve::Linear);
            this->_fogAnim->setDuration(50000);
            this->_fogAnim->setStartValue(0);
            this->_fogAnim->setEndValue(texture.width());
            this->_fogAnim->setLoopCount(-1);

        }

        ~MapViewFog() {
            this->_fogAnim->deleteLater();
        }

        void triggerAnimation() override {
            this->_fogAnim->start();
        }

        QRectF QGraphicsItem::boundingRect() const override {
            return QRectF();
        }

        qreal textureHPos() const {
            return this->_brush.transform().m31();
        }

        void drawToPoint(const QPointF &dest) {
            this->_drawnPath.lineTo(dest);
        }

        QPainterPath commitDrawing() {
            auto out = this->_drawnPath;
            this->_drawnPath.clear();
            return out;
        }

        void setTextureHPos(qreal newPos) {
            QTransform translated;
            translated.translate(newPos, 0);
            this->_brush.setTransform(translated);
            this->update();
        }

        void defineRectFromView(QGraphicsView *view) {
            this->_viewRect = view->geometry();
        }

        //
        
        void setFogMode(const RPZFogParams::Mode &mode, bool shouldUpdate = true) {
            this->_mode = mode;
            this->_updatedComputedPath();
            if(shouldUpdate) this->update();
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            painter->save();

                auto clipPath = this->_computedPath;
                clipPath.addPath(this->_drawnPath);

                painter->setClipPath(clipPath);

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(this->_brush);
                    painter->setTransform(QTransform());
                    painter->setOpacity(AppContext::fogOpacity());

                    QPainterPath p;
                    painter->drawRect(this->_viewRect);

            painter->restore();

        }

    private:
        RPZFogParams::Mode _mode;
        QRectF _viewRect;
        QPainterPath _rawPath;
        QPainterPath _drawnPath;
        QPainterPath _computedPath;
        QBrush _brush;
        QPropertyAnimation* _fogAnim = nullptr;

        void _updatedComputedPath() {
            if(this->_mode == RPZFogParams::Mode::PathIsButFog) {
                this->_computedPath = QPainterPath();
                this->_computedPath.addRect(this->scene()->sceneRect());
                this->_computedPath.addPath(this->_rawPath);
            } 
            
            else {
                this->_computedPath = this->_rawPath;
            }
        }

};