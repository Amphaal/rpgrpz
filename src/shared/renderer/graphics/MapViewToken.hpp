#pragma once

#include <QGraphicsItem>
#include <QObject>
#include <QBrush>
#include <QPen>
#include <QSizeF>
#include <QColor>
#include <QPainter>
#include <QWidget>
#include <QStyleOptionGraphicsItem>

class MapViewToken : public QObject, public QGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_INTERFACES(QGraphicsItem)

    public:
        MapViewToken(const QSizeF &size, const QColor &color) {          
            
            this->_subRect = QRectF({0,0}, size);

            auto prc = this->_subRect.width() * 0.03;
            this->_rect = this->_subRect.marginsRemoved(QMarginsF(prc, prc, prc, prc));
            
            QRadialGradient radialGrad(this->_rect.center(), this->_rect.width() / 2);
            radialGrad.setColorAt(0.95, color);
            radialGrad.setColorAt(1, Qt::transparent);
            this->_brush = QBrush(radialGrad);

            this->_subBrush = QBrush(QColor::fromRgbF(color.redF(), color.greenF(), color.blueF(), .75));
            

        }

        QRectF boundingRect() const {
            return this->_rect;
        }

    private:
        QRectF _rect;
        QRectF _subRect;

        QBrush _brush;
        QBrush _subBrush;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            painter->save();

                if(option->state.testFlag(QStyle::StateFlag::State_Selected)) {
                    painter->setPen(QPen(QBrush("#000"), 0, Qt::PenStyle::DashLine));
                    painter->drawRect(this->_subRect);
                    painter->setOpacity(.8);
                }

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                
                painter->setBrush(this->_subBrush);
                painter->drawRoundedRect(this->_subRect, 75, 75, Qt::RelativeSize);
                
                painter->setBrush(this->_brush);
                painter->drawEllipse(this->_rect);

            painter->restore();

        }
    
    

};