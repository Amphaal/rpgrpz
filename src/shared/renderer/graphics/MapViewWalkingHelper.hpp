#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QPointF>
#include <QPainter>
#include <QDebug>
#include <QSizeF>
#include <QFont>
#include <QStyleOptionGraphicsItem>
#include <QTextOption>

#include "src/helpers/StringHelper.hpp"

#include "src/helpers/_appContext.h"
#include "src/shared/hints/AtomsStorage.h"

class MapViewWalkingHelper : public QObject, public QGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_INTERFACES(QGraphicsItem)
    
    public:
        MapViewWalkingHelper(const RPZMapParameters &params, QGraphicsItem* toWalk, QGraphicsView* view) : _view(view), _toWalk(toWalk) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);
            this->setPos(toWalk->pos());
            this->setZValue(toWalk->zValue() - 1);

            this->_mapParams = params;

        }

        QRectF boundingRect() const override {
            
            auto viewCursorPos = this->_view->mapFromGlobal(QCursor::pos());
            auto sceneCursorPos = this->_view->mapToScene(viewCursorPos);
            auto itemCursorPos = this->mapFromScene(sceneCursorPos);

            auto line = QLineF({}, itemCursorPos);
            
            auto sizePart = qAbs(line.length() * 2);
            auto size = QSizeF(sizePart, sizePart);

            QRectF out({}, size);
            out.moveCenter({});

            return out;

        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {

            auto viewCursorPos = this->_view->mapFromGlobal(QCursor::pos());
            auto sceneCursorPos = this->_view->mapToScene(viewCursorPos);
            auto itemCursorPos = this->mapFromScene(sceneCursorPos);

            painter->save();
                
                QPen pen;
                pen.setWidth(0);
                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing, true);
                
                QLineF line({0,0}, itemCursorPos);
                painter->drawLine(line);

                QBrush brush(Qt::BrushStyle::SolidPattern);
                brush.setColor("#eb6e34");
                painter->setBrush(brush);

                painter->setOpacity(.8);
                painter->drawEllipse(option->exposedRect);

            painter->restore();
            
            painter->save();

                painter->setTransform(QTransform());

                QFont font;
                font.setPointSize(15);
                painter->setFont(font);
                
                QTextOption aa;
                aa.setWrapMode(QTextOption::NoWrap);

                auto meters = this->_mapParams.distanceIntoIngameMeters(line.length());
                auto text = StringHelper::fromMeters(meters);
                auto textRect = painter->boundingRect(QRectF(), text, aa);
                textRect = this->_adjustText(itemCursorPos, viewCursorPos, textRect);

                painter->drawText(textRect, text, aa);

            painter->restore();

        }

    private:
        RPZMapParameters _mapParams;
        QGraphicsView* _view = nullptr;
        QGraphicsItem* _toWalk = nullptr;

        QRectF _adjustText(const QPointF &relativeTo, QPointF dest, QRectF toAdjust) {
            
            QPointF correct;

                if(relativeTo.x() < 0) {
                    correct += {-12, 0};
                } else {
                    correct += {12, 0};
                }

                if(relativeTo.y() < 0) {
                    correct += {0, -12};
                } else {
                    correct += {0, 12};
                }

            dest += correct;

            if(correct.x() < 0 && correct.y() < 0) toAdjust.moveBottomRight(dest);
            else if(correct.x() >= 0 && correct.y() < 0) toAdjust.moveBottomLeft(dest);
            else if(correct.x() < 0 && correct.y() >= 0) toAdjust.moveTopRight(dest);
            else if(correct.x() >= 0 && correct.y() >= 0) toAdjust.moveTopLeft(dest);

            return toAdjust;

        }

};