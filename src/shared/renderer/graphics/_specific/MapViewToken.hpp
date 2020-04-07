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

#include <QGraphicsItem>
#include <QObject>
#include <QBrush>
#include <QPen>
#include <QSizeF>
#include <QColor>
#include <QPainter>
#include <QWidget>
#include <QStyleOptionGraphicsItem>
#include <QPropertyAnimation>

#include "src/shared/models/RPZMapParameters.hpp"
#include "src/shared/models/RPZAtom.h"

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"
#include "src/shared/renderer/graphics/_base/RPZAnimated.hpp"
#include "src/shared/renderer/animator/MapViewAnimator.hpp"

class MapViewTokenOutline : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
    Q_INTERFACES(QGraphicsItem)

    public:
        MapViewTokenOutline(QGraphicsItem* parentItem, QObject* parentObject, bool shouldDisplay) : QObject(parentObject), QGraphicsItem(parentItem), 
            _shouldDisplay(shouldDisplay) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);   
              
            this->_spinnerAnim = new QPropertyAnimation(this, "rotation", this);
            this->_spinnerAnim->setDuration(10000);
            this->_spinnerAnim->setStartValue(0);
            this->_spinnerAnim->setEndValue(360);
            this->_spinnerAnim->setLoopCount(-1);
            
        }

        ~MapViewTokenOutline() {
            this->_spinnerAnim->deleteLater();
        }

        void triggerAnimation(bool starts) {
            this->_spinnerAnim->stop();
            if(starts) this->_spinnerAnim->start();
        }

        void mayDisplay(bool shouldDisplay) {
            this->_shouldDisplay = shouldDisplay;
            this->triggerAnimation(shouldDisplay);
        }

        QRectF boundingRect() const override {
            return this->parentItem()->boundingRect();
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            if(!this->_shouldDisplay) return;
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

    private:
        bool _shouldDisplay = false;
        QPropertyAnimation* _spinnerAnim = nullptr;

        bool _mustDrawSelectionHelper() const override { 
            return false; 
        };
        
        bool _canBeDrawnInMiniMap() const override { 
            return false; 
        };

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            painter->save();

                painter->setRenderHint(QPainter::Antialiasing, true);

                QPen pen;
                pen.setWidth(2);
                pen.setColor(AppContext::WALKER_COLOR);
                pen.setStyle(Qt::DashLine);
                painter->setPen(pen);
                
                painter->setOpacity(1);
                painter->drawEllipse(option->rect);

            painter->restore();

        }

};


class MapViewToken : public QObject, public QGraphicsItem, public RPZGraphicsItem, public RPZAnimated {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_INTERFACES(QGraphicsItem)

    public:
        MapViewToken(const RPZMapParameters &mapParameters, const RPZAtom &atom) {           

            this->_tokenType = atom.type();

            auto tileSize = mapParameters.tileWidthInPoints();
            auto tokenSize = QSizeF(tileSize * .95, tileSize * .95);

            auto startPosComp = QPointF(-tokenSize.width() / 2, -tokenSize.height() / 2);
            this->_mainRect = QRectF(startPosComp, tokenSize);
            this->setTransformOriginPoint(startPosComp);

            auto prc = this->_mainRect.width() * 0.1;
            this->_upperRect = this->_mainRect.marginsRemoved(QMarginsF(prc, prc, prc, prc));
            
            this->_changeColor(atom);

        }

        ~MapViewToken() {
            if(this->_outline) this->_outline->deleteLater();
        }

        void setOwned(bool owned) {
            
            this->_owned = owned;
            RPZQVariant::setContextuallyOwned(this, owned);
            
            if(this->_outline) {
                this->_outline->mayDisplay(owned);
            }

            MapViewAnimator::animateVisibility(this);

        }

        void triggerAnimation() override {
            
            if(!this->_outline) {
                this->_outline = new MapViewTokenOutline(this, this, this->_owned); 
            }

            this->_outline->triggerAnimation(this->_owned);

        }

        void updateColor(const QColor &toApply) {
            this->_changeColor(toApply);
            this->update();
        }


        QRectF boundingRect() const override {
            return this->_mainRect;
        }

        QPainterPath shape() const override {
            QPainterPath path;
            path.addEllipse(this->_mainRect);
            return path;
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

    private:
        MapViewTokenOutline* _outline = nullptr;
        RPZAtom::Type _tokenType;
        bool _owned = false;

        QRectF _upperRect;
        QRectF _mainRect;

        QBrush _upperBrush;
        QBrush _mainBrush;

        bool _mustDrawSelectionHelper() const override { 
            return true; 
        };
        
        bool _canBeDrawnInMiniMap() const override { 
            return false; 
        };

        const QString _opacityPlaceholder() const override {
            return QStringLiteral(u":/assets/grass.png");
        }

        void _changeColor(const RPZAtom &atom) {
            
            QColor toApply;

            switch(atom.type()) {
                
                case RPZAtom::Type::Player: {
                    toApply = atom.defaultPlayerColor();
                }
                break;

                case RPZAtom::Type::NPC: {
                    toApply = atom.NPCAssociatedColor();
                }
                break;

                default:
                break;

            }

            this->_changeColor(toApply);

        }

        void _changeColor(const QColor &toApply) {
            
            auto mainOpacity = 1.0;

            if(this->_tokenType == RPZAtom::Type::Player) {

                //upper
                QRadialGradient radialGrad(this->_upperRect.center(), this->_upperRect.width() / 2);
                radialGrad.setColorAt(0.95, toApply);
                radialGrad.setColorAt(1, Qt::transparent);
                this->_upperBrush = QBrush(radialGrad);

                //
                mainOpacity = .75;

            }

            //main
            this->_mainBrush = QBrush(QColor::fromRgbF(
                toApply.redF(), 
                toApply.greenF(), 
                toApply.blueF(), 
                mainOpacity
            ));

        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            
            painter->save();

                painter->setRenderHint(QPainter::Antialiasing, true);

                painter->setPen(Qt::NoPen);
                
                painter->setBrush(this->_mainBrush);
                painter->drawEllipse(this->_mainRect);
                
                if(this->_tokenType == RPZAtom::Type::Player) {
                    
                    //upper
                    painter->setBrush(this->_upperBrush);
                    painter->drawEllipse(this->_upperRect);
                    
                    //sign
                    auto sign = QObject::tr("P", "player sign");
                    painter->setPen(QColor(Qt::white));

                        auto font = painter->font();
                        font.setPixelSize((int)(this->_upperRect.height() * .9));
                        painter->setFont(font);

                        QFontMetrics m(font);
                        auto signRect = QRectF(m.boundingRect(sign));
                        signRect.moveCenter(this->_upperRect.center());
                        
                    painter->drawText(signRect, sign);

                }

            //if overlapping another grid bound
            if(RPZQVariant::isOverlapping(this)) {
                painter->drawPixmap(
                    option->exposedRect.topLeft(), 
                    QPixmap(QStringLiteral(u":/icons/app/other/overlapping.png"))
                );
            }

            painter->restore();

        }

};