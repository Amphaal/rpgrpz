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

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QPropertyAnimation>

#include "src/shared/renderer/graphics/_generic/MapViewGraphicsPixmapItem.hpp"
#include "src/shared/models/RPZAtom.h"

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"
#include "src/shared/renderer/graphics/_base/RPZAnimated.hpp"

class MapViewUnscalable : public QObject, public QGraphicsItem, public RPZGraphicsItem, public RPZAnimated {
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
    Q_INTERFACES(QGraphicsItem)

 public:
        enum class RefPoint {
            Center,
            BottomCenter
        };

        explicit MapViewUnscalable(const RPZAtom &atom) {
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations, true);

            this->_type = atom.type();
            auto assetPath = _assetPathByAtomType.value(this->_type);
            this->_p = QPixmap(assetPath).scaled(QSize(48, 48), Qt::AspectRatioMode::KeepAspectRatio);

            switch (this->_type) {
                case RPZAtom::Type::POI: {
                    this->_refP = RefPoint::BottomCenter;
                }
                break;

                case RPZAtom::Type::Event: {
                    this->_refP = RefPoint::Center;
                }
                break;

                default:
                break;
            }
        }

        void triggerAnimation() override {
            // scaling
            auto scale = new QPropertyAnimation(this, "scale");
            scale->setEasingCurve(QEasingCurve::InOutQuad);
            scale->setDuration(2000);
            scale->setKeyValues({
                {0, 0.85},
                {0.5, 1},
                {1, 0.85}
            });
            scale->setLoopCount(-1);
            scale->setCurrentTime(QRandomGenerator::global()->bounded(0, scale->duration()));
            scale->start(QAbstractAnimation::DeleteWhenStopped);

            // rotation
            if (this->_type == RPZAtom::Type::Event) {
                auto rotate = new QPropertyAnimation(this, "rotation");
                rotate->setEasingCurve(QEasingCurve::Linear);
                rotate->setDuration(3000);
                rotate->setKeyValues({
                    {0, 0},
                    {1, 360}
                });
                rotate->setLoopCount(-1);
                rotate->setCurrentTime(QRandomGenerator::global()->bounded(0, rotate->duration()));
                rotate->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }

        QRectF boundingRect() const override {
            auto rect = QRectF(this->_p.rect());

            switch (this->_refP) {
                case RefPoint::Center: {
                    rect.moveCenter({});
                }
                break;

                case RefPoint::BottomCenter: {
                    rect.moveCenter({});
                    rect.moveBottom(0);
                }
                break;
            }

            return rect;
        }

 protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if (!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

        bool _mustDrawSelectionHelper() const override {
            return true;
        };

 private:
        QPixmap _p;
        MapViewUnscalable::RefPoint _refP;
        RPZAtom::Type _type;

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            painter->save();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawPixmap(this->boundingRect(), this->_p, this->_p.rect());

            painter->restore();
        }

        static const inline QHash<RPZAtom::Type, QString> _assetPathByAtomType = {
            { RPZAtom::Type::Event, QStringLiteral(u":/assets/event.png") },
            { RPZAtom::Type::POI, QStringLiteral(u":/assets/pointOfInterest.png") },
        };
};
