#pragma once

#include <QObject>
#include <QGraphicsItem>
#include <QPropertyAnimation>
#include <QGraphicsScene>
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
        MapViewFog(QGraphicsScene* sceneToBindTo) {
            
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

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {

            painter->save();

                painter->setClipPath(this->_path);

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(this->_brush);
                    painter->setTransform(QTransform());

                    QPainterPath p;
                    painter->drawRect(option->rect);

            painter->restore();

        }

        void _initTestCombo() {
            
            this->_testCb = new QComboBox;
            
            auto addOp = [=](QPainter::CompositionMode mode, QString str) {
                this->_testCb->addItem(str, (int)mode);
            };

            addOp(QPainter::CompositionMode_SourceOver, "SourceOver");
            addOp(QPainter::CompositionMode_DestinationOver, "DestinationOver");
            addOp(QPainter::CompositionMode_Clear, "Clear");
            addOp(QPainter::CompositionMode_Source, "Source");
            addOp(QPainter::CompositionMode_Destination, "Destination");
            addOp(QPainter::CompositionMode_SourceIn, "SourceIn");
            addOp(QPainter::CompositionMode_DestinationIn, "DestinationIn");
            addOp(QPainter::CompositionMode_SourceOut, "SourceOut");
            addOp(QPainter::CompositionMode_DestinationOut, "DestinationOut");
            addOp(QPainter::CompositionMode_SourceAtop, "SourceAtop");
            addOp(QPainter::CompositionMode_DestinationAtop, "DestinationAtop");
            addOp(QPainter::CompositionMode_Xor, "Xor");
            addOp(QPainter::CompositionMode_Plus, "Plus");
            addOp(QPainter::CompositionMode_Multiply, "Multiply");
            addOp(QPainter::CompositionMode_Screen, "Screen");
            addOp(QPainter::CompositionMode_Overlay, "Overlay");
            addOp(QPainter::CompositionMode_Darken, "Darken");
            addOp(QPainter::CompositionMode_Lighten, "Lighten");
            addOp(QPainter::CompositionMode_ColorDodge, "ColorDodge");
            addOp(QPainter::CompositionMode_ColorBurn, "ColorBurn");
            addOp(QPainter::CompositionMode_HardLight, "HardLight");
            addOp(QPainter::CompositionMode_SoftLight, "SoftLight");
            addOp(QPainter::CompositionMode_Difference, "Difference");
            addOp(QPainter::CompositionMode_Exclusion, "Exclusion");

            QObject::connect(
                this->_testCb, QOverload<int>::of(&QComboBox::highlighted),
                [=](int index) {
                    this->_testMode = (QPainter::CompositionMode)this->_testCb->itemData(index).toInt();
                }
            );

            this->_testCb->show();

        }

    private:
        QPainterPath _path;
        QBrush _brush;
        QPropertyAnimation* _fogAnim = nullptr;

        //test
        QComboBox* _testCb = nullptr;
        QPainter::CompositionMode _testMode = (QPainter::CompositionMode)0;

};