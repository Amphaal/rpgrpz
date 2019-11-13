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

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class MapViewToken : public QObject, public QGraphicsItem, public RPZGraphicsItem {
    
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_INTERFACES(QGraphicsItem)

    public:
        MapViewToken(const RPZMapParameters &mapParameters, const RPZAtom &atom) {          
            
            this->_tokenType = atom.type();
            this->setAcceptHoverEvents(true);

            auto tileSize = mapParameters.tileWidthInPoints();
            auto tokenSize = QSizeF(tileSize, tileSize);

            auto startPosComp = QPointF(-tokenSize.width() / 2, -tokenSize.height() / 2);
            this->_mainRect = QRectF(startPosComp, tokenSize);

            auto prc = this->_mainRect.width() * 0.1;
            this->_upperRect = this->_mainRect.marginsRemoved(QMarginsF(prc, prc, prc, prc));
            
            this->_changeColor(atom);
            
        }

        void updateColor(const QColor &toApply) {
            this->_changeColor(toApply);
            this->update();
        }


        QRectF boundingRect() const override {
            return this->_mainRect;
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

    private:
        RPZAtom::Type _tokenType;

        QRectF _upperRect;
        QRectF _mainRect;

        QBrush _upperBrush;
        QBrush _mainBrush;

        bool _drawSelectionHelper() override { 
            return true; 
        };

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
            
            auto mainOpacity = 1.0;

            if(this->_tokenType == RPZAtom::Type::Player) {

                //upper
                QRadialGradient radialGrad(this->_upperRect.center(), this->_upperRect.width() / 2);
                radialGrad.setColorAt(0.95, toApply);
                radialGrad.setColorAt(1, Qt::transparent);
                this->_upperBrush = QBrush(radialGrad);

                //
                mainOpacity = .75;

            }

            //main
            this->_mainBrush = QBrush(QColor::fromRgbF(
                toApply.redF(), 
                toApply.greenF(), 
                toApply.blueF(), 
                mainOpacity
            ));

        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            
            painter->save();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                
                painter->setBrush(this->_mainBrush);
                painter->drawRoundedRect(this->_mainRect, 75, 75, Qt::RelativeSize);
                
                if(this->_tokenType == RPZAtom::Type::Player) {
                    
                    //upper
                    painter->setBrush(this->_upperBrush);
                    painter->drawEllipse(this->_upperRect);
                    
                    //sign
                    auto sign = QObject::tr("P", "player sign");
                    painter->setPen(QColor(Qt::white));

                        auto font = painter->font();
                        font.setPixelSize((int)(this->_upperRect.height() * .9));
                        painter->setFont(font);

                        QFontMetrics m(font);
                        auto signRect = QRectF(m.boundingRect(sign));
                        signRect.moveCenter(this->_upperRect.center());
                        
                    painter->drawText(signRect, sign);

                }

            painter->restore();

        }

};