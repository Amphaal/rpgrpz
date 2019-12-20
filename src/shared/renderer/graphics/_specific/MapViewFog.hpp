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
#include "src/shared/payloads/fog/FogChangedPayload.hpp"

#include "src/helpers/_appContext.h"
#include "src/helpers/VectorSimplifier.hpp"

class MapViewFog : public QObject, public QGraphicsItem, public RPZGraphicsItem, public RPZAnimated {
    
    Q_OBJECT
    Q_PROPERTY(qreal textureHPos READ textureHPos WRITE setTextureHPos)
    Q_INTERFACES(QGraphicsItem)
    
    public:
        MapViewFog(const RPZFogParams &params) {
            
            //init from params
            this->_fogPolys = params.polys();
            this->setFogMode(params.mode());

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

        void setFogMode(const RPZFogParams::Mode &mode) {
            this->_mode = mode;
            this->_updateComputedPath();
        }

        void triggerAnimation() override {
            this->_fogAnim->start();
        }

        QRectF boundingRect() const override {
            return this->scene() ? this->scene()->sceneRect() : QRectF();
        }

        qreal textureHPos() const {
            return this->_brush.transform().m31();
        }

        void setTextureHPos(qreal newPos) {
            QTransform translated;
            translated.translate(newPos, 0);
            this->_brush.setTransform(translated);
            this->update();
        }

        ///
        ///
        ///

        void initDrawing(const FogChangedPayload::ChangeType &type) {
            this->_drawnOpe = type;
        }

        void drawToPoint(const QPointF &dest) {
            this->_drawnPoly << dest;
        }

        QList<QPolygonF> commitDrawing() {

            auto reduced = VectorSimplifier::reducePolygon(this->_drawnPoly);
            auto simplified = VectorSimplifier::simplifyPolygon(reduced);

            this->_clearDrawing();

            return simplified;

        }

        void updateFog(const QList<QPolygonF> &polys) {
            this->_fogPolys = polys;
            this->_updateComputedPath();
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
                clipPath.addPolygon(this->_drawnPoly);

                painter->setClipPath(clipPath);

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(this->_brush);
                    painter->setTransform(QTransform());
                    painter->setOpacity(AppContext::fogOpacity());
                    painter->drawRect(this->scene()->sceneRect());

            painter->restore();

        }

    private:
        RPZFogParams::Mode _mode;
        QPainterPath _computedPath;
        QBrush _brush;
        QPropertyAnimation* _fogAnim = nullptr;
        
        FogChangedPayload::ChangeType _drawnOpe;
        QPolygonF _drawnPoly;

        QList<QPolygonF> _fogPolys;

        void _updateComputedPath() {
            //TODO move to main thread?
            this->_computedPath = QPainterPath();

            if(this->_mode == RPZFogParams::Mode::PathIsButFog) {
                this->_computedPath.addRect(this->scene()->sceneRect());   
            }
            
            for(const auto &poly : this->_fogPolys) {
                this->_computedPath.addPolygon(poly);
            }

        }

        void _clearDrawing() {
            this->_drawnPoly.clear();
        }
        
        void _clear() {
            this->_clearDrawing();
            this->_fogPolys.clear();
            this->_updateComputedPath();
        }

};