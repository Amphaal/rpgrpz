// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

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
        struct FogChangingVisibility {
            QList<QGraphicsItem*> nowVisible;
            QList<QGraphicsItem*> nowInvisible;
        };
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

        const FogChangingVisibility coveredAtomItems() {
            
            FogChangingVisibility out;
            out.nowInvisible = _atomItemsFromGraphicsItems(this->collidingItems());
            
            _logFogChangingVisibility(out);
            
            return out;

        }

        const FogChangingVisibility setFogMode(const RPZFogParams::Mode &mode) {
            
            this->_setFogMode(mode);
            
            auto out = this->_generateClipPathWithDiff();

            return out;
        }

        const FogChangingVisibility updateFog(const QList<QPolygonF> &wholeShape) {

            //reset clip shape to before drawing to get an accurate snapshot
            if(this->_drawnPoly.count()) {
                this->_clearDrawing();
                this->_generateClipPath();
            }

            //reset drawing and update fog shape
            this->_updateFog(wholeShape);
            
            //recreate clip path and diff with drawing
            auto out = this->_generateClipPathWithDiff();

            return out;
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

        const QList<QGraphicsItem*> _atomItemsFromGraphicsItems(const QList<QGraphicsItem*> &toFilter) const {
            
            QList<QGraphicsItem*> cAtomItems;
            
            for(auto item : toFilter) {
                if(!dynamic_cast<RPZGraphicsItem*>(item)) continue; //only for main RPZGraphicsItem
                if(item->parentItem()) continue; //must not have a parent
                if(RPZQVariant::isTemporary(item)) continue;
                cAtomItems.append(item); 
            }
            
            return cAtomItems;

        }
        const QList<QGraphicsItem*> _atomItemsFromGraphicsItems(const QSet<QGraphicsItem*> &toFilter) const {
            return _atomItemsFromGraphicsItems(QList<QGraphicsItem*>(toFilter.begin(), toFilter.end()));
        }

        void _clearDrawing() {
            this->_drawnPoly.clear();
        }

        void _updateFog(const QList<QPolygonF> &wholeShape) {
            
            this->_fog.clear();
            
            for(auto const &poly : wholeShape) {
                this->_fog.addPolygon(poly);
            }

        }

        void _setFogMode(const RPZFogParams::Mode &mode) {
            this->_mode = mode;
        }
        
        QSet<QGraphicsItem*> _snapshotColliding() const {
            auto colliding = this->collidingItems();
            auto collidingSet = QSet<QGraphicsItem*>(colliding.begin(), colliding.end());
            return collidingSet;
        }

        const FogChangingVisibility _generateClipPathWithDiff() {
            
            FogChangingVisibility out;

            auto thenColliding = _snapshotColliding();
            this->_generateClipPath();
            auto nowColliding = _snapshotColliding();

            //sort out visible and invisible
            auto nowInvisible = nowColliding;
            nowInvisible.subtract(thenColliding);
            auto nowVisible = thenColliding;
            nowVisible.subtract(nowColliding);

            out.nowInvisible = _atomItemsFromGraphicsItems(nowInvisible);
            out.nowVisible = _atomItemsFromGraphicsItems(nowVisible);

            _logFogChangingVisibility(out);

            return out;

        }

        void _logFogChangingVisibility(const FogChangingVisibility &toLog) {
            qDebug() << qUtf8Printable(
                QStringLiteral(u"Fog Updated : %1 visibles, %2 invisibles")
                        .arg(toLog.nowVisible.count())
                        .arg(toLog.nowInvisible.count())
            );
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