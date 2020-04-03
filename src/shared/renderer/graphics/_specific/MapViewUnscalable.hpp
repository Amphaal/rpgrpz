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

#include "src/shared/renderer/graphics/_generic/MapViewGraphicsPixmapItem.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class MapViewUnscalable : public QGraphicsItem, public RPZGraphicsItem {

    Q_INTERFACES(QGraphicsItem)

    public:
        enum class RefPoint {
            Center,
            BottomCenter
        };

        MapViewUnscalable(const RPZAtom &atom) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations, true);

            auto type = atom.type();
            auto pathToIcon = RPZAtom::iconPathByAtomType.value(type);
            this->_p = QPixmap(pathToIcon);

            switch(type) {

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

        QRectF boundingRect() const override {
            auto rect = QRectF(this->_p.rect());
            
            switch(this->_refP) {
                
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
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

        bool _mustDrawSelectionHelper() const override { 
            return true; 
        };

    private:
        QPixmap _p;
        RefPoint _refP;

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            
            painter->save();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawPixmap(this->boundingRect(), this->_p, this->_p.rect());

            painter->restore();

        }

};