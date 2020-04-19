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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QPropertyAnimation>
#include <QQueue>

#include "src/shared/renderer/graphics/MapViewGraphics.h"

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

    static void setAnimationsAllowed(bool allowed) {
        _animationsAllowed = allowed;
    }

    static void animateVisibility(QGraphicsItem* toAnimate) {
        auto destOpacity = _determineOpacity(toAnimate);

        // prevent animation if opacity is the same
        auto currentOpacity = toAnimate->opacity();
        if (currentOpacity == destOpacity) return;

        // check if animation is available
        auto canBeAnimated = dynamic_cast<QObject*>(toAnimate);
        if (canBeAnimated && toAnimate->scene() && _animationsAllowed) {
                _animateVisibility(canBeAnimated, currentOpacity, destOpacity);
        } else {  // fallback
            toAnimate->setOpacity(destOpacity);
        }
    }

    static void animatePath(MapViewDrawing *toAnimate, const QPainterPath &pathToAnimate) {
        if (toAnimate && toAnimate->scene() && _animationsAllowed) {
            _animatePath(toAnimate, pathToAnimate);
        }
    }

    static void animateMove(QGraphicsItem *toAnimate, const QPointF &newScenePos) {
        auto currentPos = toAnimate->pos();
        if (currentPos == newScenePos) return;

        if (currentPos.isNull()) return toAnimate->setPos(newScenePos);  // initial set, no animation

        auto canBeAnimated = dynamic_cast<QObject*>(toAnimate);

        if (canBeAnimated && toAnimate->scene() && _animationsAllowed) {
            _animateMove(canBeAnimated, toAnimate, currentPos, newScenePos);
        } else {  // fallback
            toAnimate->setPos(newScenePos);
        }
    }

    static void clearAnimations() {
        for (auto i = _ongoingAnimations.begin(); i != _ongoingAnimations.end(); i++) {
            for (auto a : i.value()) {
                a->deleteLater();
            }
        }

        _ongoingAnimations.clear();
        _queuedAnimations.clear();
    }

 private:
    static inline bool _animationsAllowed = true;

    static inline QString _visibilityProp = QStringLiteral(u"opacity");
    static inline QString _moveProp = QStringLiteral(u"pos");
    static inline QString _pathProp = QStringLiteral(u"path");

    static inline QHash<QObject*, QHash<QString, QPropertyAnimation*>> _ongoingAnimations;
    static inline QQueue<QPropertyAnimation*> _queuedAnimations;

    static double _determineOpacity(QGraphicsItem* item) {
        auto isHostAble = Authorisations::isHostAble();
        return RPZGraphicsItem::isGameHidden(item)
                    ? (isHostAble ? .5 : 0)
                    : RPZQVariant::cachedOpacity(item);
    }

    static void _animateMove(QObject* toAnimate, QGraphicsItem* toAnimate_gi, const QPointF &currentScenePos, const QPointF &newScenePos) {
        auto existingAnim = _ongoingAnimations[toAnimate].value(_moveProp);

        // no running animation found...
        if (!existingAnim) {
            existingAnim = new QPropertyAnimation(toAnimate, _moveProp.toLocal8Bit());
            _ongoingAnimations[toAnimate].insert(_moveProp, existingAnim);

            RPZQVariant::setMoveAnimationDestinationScenePoint(toAnimate_gi, newScenePos);

            existingAnim->setDuration(400);
            existingAnim->setEasingCurve(QEasingCurve::InQuad);
            existingAnim->setStartValue(currentScenePos);
            existingAnim->setEndValue(newScenePos);

            QObject::connect(
                existingAnim, &QAbstractAnimation::finished,
                [=]() {
                    RPZQVariant::deleteMoveAnimationDestinationScenePoint(toAnimate_gi);
                    _clearAnimation(toAnimate, _moveProp);
            });

            _queuedAnimations.enqueue(existingAnim);

        } else {  // animation already exists
            existingAnim->pause();
            existingAnim->setStartValue(currentScenePos);
            existingAnim->setEndValue(newScenePos);
            existingAnim->start();
        }
    }

    static void _animateVisibility(QObject *toAnimate, qreal currentOpacity, qreal destOpacity) {
        auto existingAnim = _ongoingAnimations[toAnimate].value(_visibilityProp);

        // no running animation found...
        if (!existingAnim) {
            existingAnim = new QPropertyAnimation(toAnimate, _visibilityProp.toLocal8Bit());
            _ongoingAnimations[toAnimate].insert(_visibilityProp, existingAnim);

            existingAnim->setDuration(500);
            existingAnim->setStartValue(currentOpacity);
            existingAnim->setEndValue(destOpacity);

            QObject::connect(
                existingAnim, &QAbstractAnimation::finished,
                [=]() {
                    _clearAnimation(toAnimate, _visibilityProp);
            });

            _queuedAnimations.enqueue(existingAnim);

        } else {  // animation already exists
            existingAnim->pause();
            existingAnim->setStartValue(currentOpacity);
            existingAnim->setEndValue(destOpacity);
            existingAnim->start();
        }
    }

    static void _animatePath(MapViewDrawing* toAnimate, const QPainterPath &pathToAnimate) {
        auto existingAnim = _ongoingAnimations[toAnimate].value(_pathProp);

        // no running animation found...
        if (!existingAnim) {
            existingAnim = new PathAnimator(toAnimate, _pathProp.toLocal8Bit());
            _ongoingAnimations[toAnimate].insert(_pathProp, existingAnim);

            existingAnim->setDuration(250);
            existingAnim->setEasingCurve(QEasingCurve::InQuad);
            existingAnim->setStartValue(QVariant::fromValue<QPainterPath>(QPainterPath()));
            existingAnim->setEndValue(QVariant::fromValue<QPainterPath>(pathToAnimate));

            QObject::connect(
                existingAnim, &QAbstractAnimation::finished,
                [=]() {
                    _clearAnimation(toAnimate, _pathProp);
            });

            _queuedAnimations.enqueue(existingAnim);
        } else {  // animation already exists
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
        // remove animation ref
        auto anim = _ongoingAnimations[objectToClearOf].take(propToClear);

        // remove QObject reference alltogether if no more animations
        if (!_ongoingAnimations[objectToClearOf].count()) _ongoingAnimations.remove(objectToClearOf);

        // remove the handler
        anim->deleteLater();
    }
};
Q_DECLARE_METATYPE(QPainterPath)
