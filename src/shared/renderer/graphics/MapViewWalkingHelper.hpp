#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QPointF>
#include <QPainter>
#include <QDebug>
#include <QSizeF>
#include <QFont>
#include <QStyleOptionGraphicsItem>

class MapViewWalkingHelper : public QObject, public QGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_INTERFACES(QGraphicsItem)
    
    public:
        MapViewWalkingHelper(QGraphicsItem* toWalk, QGraphicsView* view) {
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);
            this->setPos(toWalk->pos());
            this->setZValue(toWalk->zValue() - 1);
            this->_view = view;
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

                auto text = QString::number(line.length());

                auto displayAt = viewCursorPos;
                displayAt += {12, 0};
                painter->drawText(displayAt, text);

            painter->restore();

        }

    private:
        QGraphicsView* _view = nullptr;

};