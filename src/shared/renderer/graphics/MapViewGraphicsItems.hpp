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

        static void clearAnimations() {
            
            for(auto &animations : _ongoingAnimations) {
                qDeleteAll(animations);
            }

            _ongoingAnimations.clear();

        }
    
    private:
        static inline QHash<QObject*, QHash<QString, QPropertyAnimation*>> _ongoingAnimations;
        static inline QString _opacityProp = QStringLiteral(u"opacity");

        static void _animateVisibility(QObject *toAnimate, qreal currentOpacity, qreal destOpacity) {

            auto existingAnim = _ongoingAnimations[toAnimate].value(_opacityProp);

            //no running animation found...
            if(!existingAnim) {
                existingAnim = new QPropertyAnimation(toAnimate, _opacityProp.toLocal8Bit());
                existingAnim->setDuration(500);
                existingAnim->setStartValue(currentOpacity);
                existingAnim->setEndValue(destOpacity);
                existingAnim->start();

                QObject::connect(
                    existingAnim, &QAbstractAnimation::finished,
                    [=]() {
                        
                        //remove animation ref
                        _ongoingAnimations[toAnimate].remove(_opacityProp);

                        //remove QObject reference alltogether if no more animations
                        if(!_ongoingAnimations[toAnimate].count()) _ongoingAnimations.remove(toAnimate);

                        //remove the handler
                        existingAnim->deleteLater();

                    }
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
};