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

#include "src/shared/models/RPZMapParameters.hpp"
#include "src/shared/models/RPZAtom.h"

class MapViewToken : public QObject, public QGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_INTERFACES(QGraphicsItem)

    public:
        MapViewToken(const RPZMapParameters &mapParameters, const RPZAtom &atom) {          
            
            this->setAcceptHoverEvents(true);

            auto tileSize = mapParameters.tileWidthInPoints();
            auto tokenSize = QSizeF(tileSize, tileSize);

            auto startPosComp = QPointF(-tokenSize.width() / 2, -tokenSize.height() / 2);
            this->_subRect = QRectF(startPosComp, tokenSize);

            auto prc = this->_subRect.width() * 0.1;
            this->_rect = this->_subRect.marginsRemoved(QMarginsF(prc, prc, prc, prc));
            
            this->_changeColor(atom);
            
        }

        void updateColor(const QColor &toApply) {
            this->_changeColor(toApply);
            this->update();
        }


        QRectF boundingRect() const override {
            return this->_subRect;
        }

    private:
        QRectF _rect;
        QRectF _subRect;

        QBrush _brush;
        QBrush _subBrush;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            
            if(option->state.testFlag(QStyle::StateFlag::State_Selected)) {
                painter->save();
                    
                    QPen pen;
                    pen.setWidth(0);
                    pen.setStyle(Qt::DashLine);
                    painter->setPen(pen);

                    painter->drawRect(option->exposedRect);

                painter->restore();
            }

            painter->save();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                
                painter->setBrush(this->_subBrush);
                painter->drawRoundedRect(this->_subRect, 75, 75, Qt::RelativeSize);
                
                painter->setBrush(this->_brush);
                painter->drawEllipse(this->_rect);

            painter->restore();

        }

        void _changeColor(const RPZAtom &atom) {
            
            QColor toApply;

            switch(atom.type()) {
                
                case RPZAtom::Type::Player: {
                    toApply = atom.defaultPlayerColor();
                }
                break;

                case RPZAtom::Type::NPC: {
                    toApply = atom.NPCAssociatedColor();
                }
                break;

                default:
                break;

            }

            this->_changeColor(toApply);

        }

        void _changeColor(const QColor &toApply) {
            
            //main
            QRadialGradient radialGrad(this->_rect.center(), this->_rect.width() / 2);
            radialGrad.setColorAt(0.95, toApply);
            radialGrad.setColorAt(1, Qt::transparent);
            this->_brush = QBrush(radialGrad);

            //sub
            this->_subBrush = QBrush(QColor::fromRgbF(
                toApply.redF(), 
                toApply.greenF(), 
                toApply.blueF(), 
                .75
            ));

        }
    
    

};