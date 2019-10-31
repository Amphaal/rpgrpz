#pragma once

#include "MapViewGraphicsPathItem.hpp"
#include "MapViewGraphicsPixmapItem.hpp"
#include "MapViewGraphicsRectItem.hpp"
#include "MapViewGraphicsSvgItem.hpp"
#include "MapViewGraphicsTextItem.hpp"

#include <QPropertyAnimation>

class MapViewGraphicsItems {
    public:
        static void animateVisibility(QGraphicsItem *toAnimate, bool isHidden) {
            
            auto currentOpacity = toAnimate->opacity();
            auto destOpacity = isHidden ? 0.0 : 1.0;
            if(currentOpacity == destOpacity) return;

            if(auto canBeAnimated = dynamic_cast<QObject*>(toAnimate)) {
                _animateVisibility(canBeAnimated, currentOpacity, destOpacity);
            } 

            //fallback
            else {
                toAnimate->setOpacity(destOpacity);
            }

        }

        static void animateMove(QGraphicsItem *toAnimate, const QPointF &newScenePos) {
            
            auto currentPos = toAnimate->pos();
            if(currentPos == newScenePos) return;
            
            if(currentPos.isNull()) return toAnimate->setPos(newScenePos); //initial set, no animation

            if(auto canBeAnimated = dynamic_cast<QObject*>(toAnimate)) {
                _animateMove(canBeAnimated, currentPos, newScenePos);
            } 

            //fallback
            else {
                toAnimate->setPos(newScenePos);
            }

        }

        static void clearAnimations() {
            
            for(auto &animations : _ongoingAnimations) {
                qDeleteAll(animations);
            }

            _ongoingAnimations.clear();

        }
    
    private:
        static inline QString _visibilityProp = QStringLiteral(u"opacity");
        static inline QString _moveProp = QStringLiteral(u"pos");
        
        static inline QHash<QObject*, QHash<QString, QPropertyAnimation*>> _ongoingAnimations;

        static void _animateMove(QObject *toAnimate, const QPointF &currentScenePos, const QPointF &newScenePos) {
            
            auto existingAnim = _ongoingAnimations[toAnimate].value(_moveProp);

            //no running animation found...
            if(!existingAnim) {
                existingAnim = new QPropertyAnimation(toAnimate, _moveProp.toLocal8Bit());
                existingAnim->setDuration(250);
                existingAnim->setEasingCurve(QEasingCurve::InQuad);
                existingAnim->setStartValue(currentScenePos);
                existingAnim->setEndValue(newScenePos);
                existingAnim->start();

                QObject::connect(
                    existingAnim, &QAbstractAnimation::finished,
                    [=]() { _clearAnimation(toAnimate, _moveProp); }
                );

            }

            //animation already exists
            else {
                existingAnim->pause();
                existingAnim->setStartValue(currentScenePos);
                existingAnim->setEndValue(newScenePos);
                existingAnim->start();
            }

        }

        static void _animateVisibility(QObject *toAnimate, qreal currentOpacity, qreal destOpacity) {

            auto existingAnim = _ongoingAnimations[toAnimate].value(_visibilityProp);

            //no running animation found...
            if(!existingAnim) {
                existingAnim = new QPropertyAnimation(toAnimate, _visibilityProp.toLocal8Bit());
                existingAnim->setDuration(500);
                existingAnim->setStartValue(currentOpacity);
                existingAnim->setEndValue(destOpacity);
                existingAnim->start();

                QObject::connect(
                    existingAnim, &QAbstractAnimation::finished,
                    [=]() { _clearAnimation(toAnimate, _visibilityProp); }
                );

            }

            //animation already exists
            else {
                existingAnim->pause();
                existingAnim->setStartValue(currentOpacity);
                existingAnim->setEndValue(destOpacity);
                existingAnim->start();
            }

        }

        //
        //
        //

        static void _clearAnimation(QObject* objectToClearOf, const QString &propToClear) {
            
            //remove animation ref
            auto anim = _ongoingAnimations[objectToClearOf].take(propToClear);

            //remove QObject reference alltogether if no more animations
            if(!_ongoingAnimations[objectToClearOf].count()) _ongoingAnimations.remove(objectToClearOf);

            //remove the handler
            anim->deleteLater();

        }
};