#pragma once

#include <QObject>
#include <QGraphicsItem>
#include <QPropertyAnimation>
#include <QGraphicsView>
#include <QComboBox>
#include <QPainterPath>

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"
#include "src/shared/renderer/graphics/_base/RPZAnimated.hpp"

#include "src/helpers/_appContext.h"

class MapViewFog : public QObject, public QGraphicsItem, public RPZGraphicsItem, public RPZAnimated {
    
    Q_OBJECT
    Q_PROPERTY(qreal textureHPos READ textureHPos WRITE setTextureHPos)
    Q_INTERFACES(QGraphicsItem)
    
    public:
        MapViewFog(QGraphicsView* view) : _view(view) {
            
            this->setZValue(AppContext::FOG_Z_INDEX);

            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable, false);  

            QPixmap texture(":/assets/smoke.png");
            this->_brush.setTexture(texture);

            this->_fogAnim = new QPropertyAnimation(this, "textureHPos", this);
            this->_fogAnim->setEasingCurve(QEasingCurve::Linear);
            this->_fogAnim->setDuration(50000);
            this->_fogAnim->setStartValue(0);
            this->_fogAnim->setEndValue(texture.width());
            this->_fogAnim->setLoopCount(-1);

        }

        ~MapViewFog() {
            this->_fogAnim->deleteLater();
        }

        void triggerAnimation() override {
            this->_fogAnim->start();
        }

        QRectF QGraphicsItem::boundingRect() const override {
            return QRectF();
        }

        qreal textureHPos() const {
            return this->_brush.transform().m31();
        }

        void setTextureHPos(qreal newPos) {
            QTransform translated;
            translated.translate(newPos, 0);
            this->_brush.setTransform(translated);
            this->update();
        }

        //
        
        void setOpacity(qreal value) {
            this->_fogOpacity = value;
            this->update();
        }

        void setReversedMode(bool isReversed) {
            
            if(isReversed) {
                this->_computedPath = QPainterPath();
                this->_computedPath.addRect(this->scene()->sceneRect());
                this->_computedPath.addPath(this->_rawPath);
            } 
            
            else {
                this->_computedPath = this->_rawPath;
            }

            this->_reversedMode = isReversed;
            this->update();
        
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            painter->save();

                painter->setClipPath(this->_computedPath);

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(this->_brush);
                    painter->setTransform(QTransform());
                    painter->setOpacity(this->_fogOpacity);

                    QPainterPath p;
                    painter->drawRect(this->_view->geometry());

            painter->restore();

        }

    private:
        QGraphicsView* _view = nullptr;
        QPainterPath _rawPath;
        QPainterPath _computedPath;
        QBrush _brush;
        QPropertyAnimation* _fogAnim = nullptr;
        qreal _fogOpacity = .5;
        bool _reversedMode = false;

};