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
    
    private:
        RPZMapParameters _mapParams;
        QGraphicsView* _view = nullptr;
        QGraphicsItem* _toWalk = nullptr;

        struct PointPos {
            QPoint viewCursorPos;
            QPointF sceneCursorPos;
            QPointF itemCursorPos;
        };

        const MapViewWalkingHelper::PointPos _generatePointPos() const {
            
            PointPos out;

            out.viewCursorPos = this->_view->mapFromGlobal(QCursor::pos());
            out.sceneCursorPos = this->_view->mapToScene(out.viewCursorPos);
            out.itemCursorPos = this->mapFromScene(out.sceneCursorPos);

            return out;

        }

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

  void _drawRangeEllipse(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewWalkingHelper::PointPos &pp) {
            
            painter->save();
                
                QPen pen;
                pen.setWidth(0);
                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing, true);
                
                QLineF line({0,0}, pp.itemCursorPos);
                painter->drawLine(line);

                QBrush brush(Qt::BrushStyle::SolidPattern);
                brush.setColor("#eb6e34");
                painter->setBrush(brush);

                painter->setOpacity(.8);
                painter->drawEllipse(option->exposedRect);

            painter->restore();

        }

        void _drawRangeTextIndicator(QPainter *painter, const QStyleOptionGraphicsItem *option, const MapViewWalkingHelper::PointPos &pp) {
            
            painter->save();

                QLineF line({0,0}, pp.itemCursorPos);
                painter->setTransform(QTransform());

                QFont font;
                font.setPointSize(15);
                painter->setFont(font);
                
                QTextOption aa;
                aa.setWrapMode(QTextOption::NoWrap);

                auto meters = this->_mapParams.distanceIntoIngameMeters(line.length());
                auto text = StringHelper::fromMeters(meters);
                auto textRect = painter->boundingRect(QRectF(), text, aa);
                textRect = this->_adjustText(pp.itemCursorPos, pp.viewCursorPos, textRect);

                painter->drawText(textRect, text, aa);

            painter->restore();

        }

        const QRectF _ellispeBoundingRect() const {
            
            auto pp = this->_generatePointPos();

            auto line = QLineF({}, pp.itemCursorPos);
            
            auto sizePart = qAbs(line.length() * 2);
            auto size = QSizeF(sizePart, sizePart);

            QRectF out({}, size);
            out.moveCenter({});

            return out;

        }

        const QRectF _gridBoundingRect() const {
            auto pp = this->_generatePointPos();
            return QRectF({}, pp.itemCursorPos).normalized();
        }

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
            
            switch(this->_mapParams.movementSystem()) {

                case RPZMapParameters::MovementSystem::Linear: {
                    return this->_ellispeBoundingRect();
                }
                break;

                case RPZMapParameters::MovementSystem::Grid: {
                    return this->_gridBoundingRect();
                }
                break;

            }
            
            return QRectF();

        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {

            auto pp = this->_generatePointPos();

            switch(this->_mapParams.movementSystem()) {

                case RPZMapParameters::MovementSystem::Linear: {
                    this->_drawRangeEllipse(painter, option, pp); //draw ellipse
                    this->_drawRangeTextIndicator(painter, option, pp); //print range indicator
                }
                break;

                case RPZMapParameters::MovementSystem::Grid: {
                    //TODO
                }
                break;

            }

        }

};