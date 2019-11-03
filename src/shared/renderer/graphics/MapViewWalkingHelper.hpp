#pragma once

#include <QGraphicsItem>
#include <QPointF>
#include <QPainter>
#include <QDebug>
#include <QSizeF>

class MapViewWalkingHelper : public QGraphicsItem {
    
    Q_INTERFACES(QGraphicsItem)
    
    public:
        MapViewWalkingHelper(QGraphicsItem* toWalk) :_toWalk(toWalk) {
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);
            this->_updateSelfPos();
            this->setZValue(toWalk->zValue() - 1);
        }

        void updateDestinationPoint(const QPointF &dest) {
            
            this->_dest = this->mapFromScene(dest);

            auto pos = this->_updateSelfPos();
            
            auto line = QLineF(pos, this->_dest);
            
            auto sizePart = qAbs(line.length() * 2);
            auto size = QSizeF(sizePart, sizePart);

            this->_rect = QRectF({0,0}, size);
            this->_rect.moveCenter(pos);

            this->update();

        }

        QRectF boundingRect() const override {
            return this->_rect;
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            
            painter->save();
                
                QPen pen;
                pen.setWidth(0);
                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing, true);

                painter->drawLine({ {0,0}, this->_dest });

                QBrush brush(Qt::BrushStyle::SolidPattern);
                brush.setColor("#eb6e34");
                painter->setBrush(brush);

                painter->setOpacity(.8);
                painter->drawEllipse(this->_rect);

            painter->restore();
            
        }

    private:
        QGraphicsItem* _toWalk = nullptr;
        QPointF _dest;
        QRectF _rect;

        const QPointF _updateSelfPos() {
            auto pos = this->_toWalk->pos();
            this->setPos(pos);
            return this->mapFromScene(pos);
        }

};