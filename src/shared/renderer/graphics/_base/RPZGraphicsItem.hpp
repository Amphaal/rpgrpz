#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "src/helpers/_appContext.h"

class RPZGraphicsItem {
    public:
        RPZGraphicsItem() {}

        struct ConditionnalPaintingResult {
            QStyleOptionGraphicsItem options;
            bool mustContinue = true; 
        };
    
    protected:
        virtual bool _canBeDrawnInMiniMap() { 
            return true; 
        };

        virtual bool _drawSelectionHelper() { 
            return false; 
        };

        ConditionnalPaintingResult conditionnalPaint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            
            ConditionnalPaintingResult out;
            out.options = *option;

            bool isMapWidget = (void*)AppContext::mapGLWidget() == widget;
            auto isSelectedState = option->state.testFlag(QStyle::StateFlag::State_Selected);

            if(isMapWidget && this->_drawSelectionHelper() && isSelectedState) {
                this->_paintSelectionHelper(painter, option);
                out.options.state.setFlag(QStyle::StateFlag::State_Selected, false);
            }

            if(!isMapWidget && !this->_canBeDrawnInMiniMap()) out.mustContinue = false;

            return out;

        };

    private:
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