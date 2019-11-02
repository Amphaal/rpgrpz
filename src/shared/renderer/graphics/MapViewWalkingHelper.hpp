#pragma once

#include <QGraphicsItem>
#include <QPointF>
#include <QPainter>
#include <QDebug>

class MapViewWalkingHelper : public QGraphicsItem {
    
    Q_INTERFACES(QGraphicsItem)
    
    public:
        MapViewWalkingHelper(QGraphicsItem* toWalk) :_toWalk(toWalk) {
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);
            this->_updateSelfPos();
        }

        void updateDestinationPoint(const QPointF &dest) {
            
            this->_dest = dest;

            auto pos = this->_toWalk->pos();
            this->setPos(pos);

            auto line = QLineF(pos, this->_dest);
            auto rect = QRectF(pos, this->_dest);
            
            auto sizePart = qAbs(line.length() * 2);
            auto size = QSizeF(sizePart, sizePart);

            this->_rect = QRectF(this->_dest, size);
            this->_rect.moveCenter(pos);

        }

        QRectF boundingRect() const override {
            return this->_rect;
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            
            painter->save();

                auto pos = this->pos();
                
                qDebug() << pos;

                painter->drawLine({ pos, this->_dest });

            painter->restore();
            
        }

    private:
        QGraphicsItem* _toWalk = nullptr;
        QPointF _dest;
        QRectF _rect;

        void _updateSelfPos() {
            this->setPos(this->_toWalk->pos());
        }

};