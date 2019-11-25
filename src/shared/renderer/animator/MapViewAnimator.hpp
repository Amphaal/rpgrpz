#pragma once

#include "src/shared/renderer/graphics/MapViewGraphics.h"

#include <QPropertyAnimation>
#include <QQueue>

#include "PathAnimator.hpp"
#include "src/helpers/RPZQVariant.hpp"
#include "src/helpers/Authorisations.hpp"

class MapViewAnimator {
    public:
        static void triggerQueuedAnimations() {
            while (!_queuedAnimations.isEmpty()) {
                auto anim = _queuedAnimations.dequeue();
                anim->start();
            }
        }

        static void animateVisibility(QGraphicsItem* toAnimate) {
            
            auto destOpacity = _determineOpacity(toAnimate);

            //prevent animation if opacity is the same
            auto currentOpacity = toAnimate->opacity();
            if(currentOpacity == destOpacity) return;
            
            //check if animation is available
            auto canBeAnimated = dynamic_cast<QObject*>(toAnimate);
            if(canBeAnimated && toAnimate->scene()) {
                 _animateVisibility(canBeAnimated, currentOpacity, destOpacity);
            } 

            //fallback
            else {
                toAnimate->setOpacity(destOpacity);
            }

        }

        static void animatePath(QGraphicsItem *toAnimate, const QPainterPath &pathToAnimate) {
            
            auto canBeAnimated = dynamic_cast<MapViewGraphicsPathItem*>(toAnimate);

            if(canBeAnimated && toAnimate->scene()) {
                _animatePath(canBeAnimated, pathToAnimate);
            } 

        }

        static void animateMove(QGraphicsItem *toAnimate, const QPointF &newScenePos) {
            
            auto currentPos = toAnimate->pos();
            if(currentPos == newScenePos) return;
            
            if(currentPos.isNull()) return toAnimate->setPos(newScenePos); //initial set, no animation

            auto canBeAnimated = dynamic_cast<QObject*>(toAnimate);

            if(canBeAnimated && toAnimate->scene()) {
                _animateMove(canBeAnimated, currentPos, newScenePos);
            } 

            //fallback
            else {
                toAnimate->setPos(newScenePos);
            }

        }

        static void clearAnimations() {
            
            for(auto i = _ongoingAnimations.begin(); i != _ongoingAnimations.end(); i++) {
                for(auto a : i.value()) {
                    a->deleteLater();
                }
            }

            _ongoingAnimations.clear();
            _queuedAnimations.clear();

        }
    
    private:
        static inline QString _visibilityProp = QStringLiteral(u"opacity");
        static inline QString _moveProp = QStringLiteral(u"pos");
        static inline QString _pathProp = QStringLiteral(u"path");
        
        static inline QHash<QObject*, QHash<QString, QPropertyAnimation*>> _ongoingAnimations;
        static inline QQueue<QPropertyAnimation*> _queuedAnimations;

        static double _determineOpacity(QGraphicsItem* item) {
            auto isHidden = RPZQVariant::isHidden(item);
            auto cachedOpacity = RPZQVariant::cachedOpacity(item);
            return isHidden ? (Authorisations::isHostAble() ? .5 : 0) : cachedOpacity;
        }

        static void _animateMove(QObject *toAnimate, const QPointF &currentScenePos, const QPointF &newScenePos) {
            
            auto existingAnim = _ongoingAnimations[toAnimate].value(_moveProp);

            //no running animation found...
            if(!existingAnim) {

                existingAnim = new QPropertyAnimation(toAnimate, _moveProp.toLocal8Bit());
                _ongoingAnimations[toAnimate].insert(_moveProp, existingAnim);

                existingAnim->setDuration(400);
                existingAnim->setEasingCurve(QEasingCurve::InQuad);
                existingAnim->setStartValue(currentScenePos);
                existingAnim->setEndValue(newScenePos);

                QObject::connect(
                    existingAnim, &QAbstractAnimation::finished,
                    [=]() { _clearAnimation(toAnimate, _moveProp); }
                );

                _queuedAnimations.enqueue(existingAnim);

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
                _ongoingAnimations[toAnimate].insert(_visibilityProp, existingAnim);

                existingAnim->setDuration(500);
                existingAnim->setStartValue(currentOpacity);
                existingAnim->setEndValue(destOpacity);

                QObject::connect(
                    existingAnim, &QAbstractAnimation::finished,
                    [=]() { _clearAnimation(toAnimate, _visibilityProp); }
                );

                _queuedAnimations.enqueue(existingAnim);

            }

            //animation already exists
            else {
                existingAnim->pause();
                existingAnim->setStartValue(currentOpacity);
                existingAnim->setEndValue(destOpacity);
                existingAnim->start();
            }

        }

        static void _animatePath(MapViewGraphicsPathItem* toAnimate, const QPainterPath &pathToAnimate) {
            
            auto existingAnim = _ongoingAnimations[toAnimate].value(_pathProp);

            //no running animation found...
            if(!existingAnim) {

                existingAnim = new PathAnimator(toAnimate, _pathProp.toLocal8Bit());
                _ongoingAnimations[toAnimate].insert(_pathProp, existingAnim);

                existingAnim->setDuration(250);
                existingAnim->setEasingCurve(QEasingCurve::InQuad);
                existingAnim->setStartValue(QVariant::fromValue<QPainterPath>(QPainterPath()));
                existingAnim->setEndValue(QVariant::fromValue<QPainterPath>(pathToAnimate));

                QObject::connect(
                    existingAnim, &QAbstractAnimation::finished,
                    [=]() { _clearAnimation(toAnimate, _pathProp); }
                );

                _queuedAnimations.enqueue(existingAnim);

            }

            //animation already exists
            else {
                existingAnim->pause();
                existingAnim->setStartValue(QVariant::fromValue<QPainterPath>(QPainterPath()));
                existingAnim->setEndValue(QVariant::fromValue<QPainterPath>(pathToAnimate));
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
Q_DECLARE_METATYPE(QPainterPath)