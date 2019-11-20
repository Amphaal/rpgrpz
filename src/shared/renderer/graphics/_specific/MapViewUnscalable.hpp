#pragma once

#include "src/shared/renderer/graphics/_generic/MapViewGraphicsPixmapItem.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>

#include "src/shared/renderer/graphics/_base/RPZGraphicsItem.hpp"

class MapViewUnscalable : public QGraphicsItem, public RPZGraphicsItem {

    Q_INTERFACES(QGraphicsItem)

    public:
        enum class RefPoint {
            Center,
            BottomCenter
        };

        MapViewUnscalable(const RPZAtom &atom) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations, true);

            auto type = atom.type();
            auto pathToIcon = RPZAtom::iconPathByAtomType.value(type);
            this->_p = QPixmap(pathToIcon);

            switch(type) {

                case RPZAtom::Type::POI: {
                    this->_refP = RefPoint::BottomCenter;
                }
                break;

                case RPZAtom::Type::Event: {
                    this->_refP = RefPoint::Center;
                }
                break;

                default:
                break;

            }

        }

        QRectF boundingRect() const override {
            auto rect = QRectF(this->_p.rect());
            
            switch(this->_refP) {
                
                case RefPoint::Center: {
                    rect.moveCenter({});
                }
                break;

                case RefPoint::BottomCenter: {
                    rect.moveCenter({});
                    rect.moveBottom(0);
                }
                break;

            }

            return rect;
        }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            auto result = this->conditionnalPaint(this, painter, option, widget);
            if(!result.mustContinue) return;
            this->_paint(painter, &result.options, widget);
        }

        bool _mustDrawSelectionHelper() const override { 
            return true; 
        };

    private:
        QPixmap _p;
        RefPoint _refP;

        void _paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) {
            
            painter->save();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawPixmap(this->boundingRect(), this->_p, this->_p.rect());

            painter->restore();

        }

};