#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "src/helpers/_appContext.h"
#include "src/network/rpz/client/RPZClient.h"
#include "src/helpers/RPZQVariant.hpp"

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

        virtual bool _drawSelectionHelper() const { 
            return false; 
        };

        virtual const QString _opacityPlaceholder() const {
            return QStringLiteral(u":/assets/hidden.png");
        }

        ConditionnalPaintingResult conditionnalPaint(QGraphicsItem* base, QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            
            ConditionnalPaintingResult out;
            out.options = *option;

            bool isMapWidget = (void*)AppContext::mapGLWidget() == widget;
            auto isSelectedState = option->state.testFlag(QStyle::StateFlag::State_Selected);

            if(isMapWidget && this->_drawSelectionHelper() && isSelectedState) {
                this->_paintSelectionHelper(painter, option);
                out.options.state.setFlag(QStyle::StateFlag::State_Selected, false);
            }

            if(!isMapWidget && !this->_canBeDrawnInMiniMap()) out.mustContinue = false;

            if(RPZClient::isHostAble() && isMapWidget && !RPZQVariant::isTemporary(base)) this->_paintOpacityPlaceholder(painter, option);

            return out;

        };

    private:

        void _paintOpacityPlaceholder(QPainter *painter, const QStyleOptionGraphicsItem *option) {
            
            if(painter->opacity() == 1) return;

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