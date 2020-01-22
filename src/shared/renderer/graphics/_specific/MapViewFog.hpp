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
        MapViewFog(const RPZFogParams &params, const RPZMapParameters &mapParams) {
            
            //init from params

            this->_maxSizeFog = mapParams.sceneRect();
            
            this->_setFogMode(params.mode());
            this->_updateFog(params.polys());
            this->_generateClipPath(); 

            this->setZValue(AppContext::FOG_Z_INDEX);

            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);  

            this->_texture = QPixmap(":/assets/smoke.png");
            this->_brush.setTexture(this->_texture);

        }

        ~MapViewFog() {
            this->_fogAnim->deleteLater();
        }

        void setFogMode(const RPZFogParams::Mode &mode) {
            this->_setFogMode(mode);
            this->_generateClipPath();
        }

        void triggerAnimation() override {

            this->_fogAnim = new QPropertyAnimation(this, "textureHPos");
            this->_fogAnim->setEasingCurve(QEasingCurve::Linear);
            this->_fogAnim->setDuration(50000);
            this->_fogAnim->setStartValue(0);
            this->_fogAnim->setEndValue(this->_texture.width());
            this->_fogAnim->setLoopCount(-1);

            this->_fogAnim->start();
        }

        QRectF boundingRect() const override {
            return this->_maxSizeFog;
        }

        QPainterPath shape() const override {
            return this->_clipPath;
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
            this->_clearDrawing();
        }

        void drawToPoint(const QPointF &dest) {
            this->_drawnPoly << dest;
            this->_generateClipPath();
        }

        QList<QPolygonF> commitDrawing() {
            
            //return empty if too small
            if(this->_drawnPoly.count() <= 3) return {};

            //reduce...
            auto reduced = VectorSimplifier::reducePolygon(this->_drawnPoly);
            
            //simplify
            auto simplified = VectorSimplifier::simplifyPolygon(reduced);

            //return drawing
            return simplified;

        }

        void updateFog(const QList<QPolygonF> &polys) {
            this->_clearDrawing();
            this->_updateFog(polys);
            this->_generateClipPath();
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            painter->save();

                    painter->setClipPath(this->_clipPath);

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(this->_brush);
                    painter->setTransform(QTransform());
                    painter->setOpacity(Authorisations::isHostAble() ? AppContext::fogOpacity() : 1);
                    painter->drawRect(this->_maxSizeFog);

            painter->restore();

        }

    private:
        QPixmap _texture;
        RPZFogParams::Mode _mode;
        QBrush _brush;
        QPropertyAnimation* _fogAnim = nullptr;
        
        FogChangedPayload::ChangeType _drawnOpe;
        QRectF _maxSizeFog;

        QPainterPath _fog;
        QPainterPath _clipPath;

        QPolygonF _drawnPoly;

        void _clearDrawing() {
            this->_drawnPoly.clear();
        }

        void _updateFog(const QList<QPolygonF> &polys) {
            
            this->_fog.clear();
            
            for(auto const &poly : polys) {
                this->_fog.addPolygon(poly);
            }

        }

        void _setFogMode(const RPZFogParams::Mode &mode) {
            this->_mode = mode;
        }

        void _generateClipPath() {

            QPainterPath clipP;

            if(this->_mode == RPZFogParams::Mode::PathIsFog) {
                clipP = this->_fog;
            } else {
                clipP.addRect(_maxSizeFog);
                clipP = clipP.subtracted(this->_fog);
            }

            if(this->_drawnPoly.count()) {
                
                QPainterPath dp;
                dp.addPolygon(this->_drawnPoly);
                
                auto mustUnite = this->_drawnOpe == FogChangedPayload::ChangeType::Added;
                if(this->_mode != RPZFogParams::Mode::PathIsFog) mustUnite = !mustUnite;

                if(mustUnite) {
                    clipP = clipP.united(dp);
                } else {
                    clipP = clipP.subtracted(dp);
                }

            }

            this->_clipPath = clipP;

        }


};