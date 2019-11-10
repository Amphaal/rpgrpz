#pragma once

#include "src/shared/renderer/graphics/_items/MapViewGraphicsPixmapItem.hpp"
#include "src/shared/models/RPZAtom.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>

class MapViewUnscalable : public QGraphicsItem {

    Q_INTERFACES(QGraphicsItem)

    public:
        MapViewUnscalable(const RPZAtom &atom) {
            
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations, true);

            auto pathToIcon = RPZAtom::iconPathByAtomType.value(atom.type());
            this->_p = QPixmap(pathToIcon);

        }

        QRectF boundingRect() const override {
            auto rect = QRectF(this->_p.rect());
            rect.moveCenter({});
            return rect;
        }

    private:
        QPixmap _p;

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
                painter->drawPixmap(this->boundingRect(), this->_p, this->_p.rect());

            painter->restore();

        }

};