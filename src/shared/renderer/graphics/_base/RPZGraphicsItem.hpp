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

#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "src/helpers/_appContext.h"
#include "src/helpers/RPZQVariant.hpp"
#include "src/helpers/Authorisations.hpp"

class RPZGraphicsItem {
    public:
        RPZGraphicsItem() {}

        struct ConditionnalPaintingResult {
            QStyleOptionGraphicsItem options;
            bool mustContinue = true; 
        };
    
    protected:
        virtual bool _canBeDrawnInMiniMap() const { 
            return true; 
        };

        virtual bool _mustDrawSelectionHelper() const { 
            return false; 
        };

        virtual const QString _opacityPlaceholder() const {
            return QStringLiteral(u":/assets/hidden.png");
        }

        ConditionnalPaintingResult conditionnalPaint(QGraphicsItem* base, QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            
            //init
            ConditionnalPaintingResult out;
            out.options = *option;

            //prepare
            bool isMapWidget = (void*)AppContext::mapGLWidget() == widget;
            auto isSelectedState = option->state.testFlag(QStyle::StateFlag::State_Selected);

            //if must print selection helper
            if(isMapWidget && isSelectedState && this->_mustDrawSelectionHelper()) {
                this->_paintSelectionHelper(painter, option);
                out.options.state.setFlag(QStyle::StateFlag::State_Selected, false); //prevent reprinting by the engine
            }

            //can continue with expected paint() from inheritor
            if(!isMapWidget && !this->_canBeDrawnInMiniMap()) out.mustContinue = false;

            //can fill with opacity placeholder
            if(isMapWidget && Authorisations::isHostAble() && !RPZQVariant::isTemporary(base)) {
                this->_paintOpacityPlaceholder(base, painter, option);
            }

            return out;

        };

    private:

        void _paintOpacityPlaceholder(QGraphicsItem* base, QPainter *painter, const QStyleOptionGraphicsItem *option) {
            
            if(!RPZQVariant::isHidden(base) && !RPZQVariant::isCoveredByFog(base)) return;

            painter->save();

                painter->setOpacity(1);

                painter->drawPixmap(option->rect, QPixmap(this->_opacityPlaceholder()));

            painter->restore();

        }

        void _paintSelectionHelper(QPainter *painter, const QStyleOptionGraphicsItem *option) {

            painter->save();
                
                QPen pen;
                pen.setWidth(1);
                pen.setCosmetic(true);
                pen.setStyle(Qt::DashLine);
                painter->setPen(pen);

                painter->drawRect(option->exposedRect);

            painter->restore();

         }

};