// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QTimeLine>
#include <QHash>

#include <functional>

class AnimationTimeLine : public QTimeLine {
 public:
    enum class Type { Zoom };
    Q_ENUM(Type)

    void setStateModifier(const qreal modifier) { this->_currentModifier = modifier; }

    static AnimationTimeLine* use(
        const AnimationTimeLine::Type &controllerType,
        qreal stateModifier,
        std::function<void(qreal, qreal)> onValueChanged
    ) {
        AnimationTimeLine* handler = nullptr;

        // create from animation type
        if (!_handlers.contains(controllerType)) {
            // create
            handler = new AnimationTimeLine(stateModifier, onValueChanged);

            // add
            _handlers.insert(controllerType, handler);
        } else {
            // get existing one and update modifier
            handler = _handlers.value(controllerType);
            handler->setStateModifier(stateModifier);
        }

        // start or restart handler
        if (handler->state() != QTimeLine::State::Running) {
            handler->start();
        } else {
            handler->stop();
            handler->setCurrentTime(handler->duration());
            handler->start();
        }

        return handler;
    }

 private:
    // static
    static inline QHash<AnimationTimeLine::Type, AnimationTimeLine*> _handlers;

    AnimationTimeLine(qreal stateModifier, std::function<void(qreal, qreal)> onValueChanged) : QTimeLine(300), _currentModifier(stateModifier) {
        // set to 30fps
        this->setUpdateInterval(10);
        this->setDirection(QTimeLine::Direction::Backward);
        this->setEasingCurve(QEasingCurve::OutCurve);

        // bind
        QObject::connect(
            this, &QTimeLine::valueChanged,
            [=](qreal value) {
                onValueChanged(this->_currentModifier, value);
        });
    }

    qreal _currentModifier = 0;
};

inline uint qHash(const AnimationTimeLine::Type &key, uint seed = 0) {return uint(key) ^ seed;}
