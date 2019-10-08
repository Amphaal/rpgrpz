#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QRect>

#include "src/shared/models/RPZUser.h"
#include "src/ui/sheets/components/Gauge.hpp"

class PlayerItemDelegate : public QStyledItemDelegate {
    public:
        PlayerItemDelegate() { }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            
            auto user = this->_getUser(index);

            switch(user.role()) {
                
                case RPZUser::Role::Observer:
                case RPZUser::Role::Host: {
                    QStyledItemDelegate::paint(painter, option, index);
                }
                break;

                
                case RPZUser::Role::Player: {
                    
                    auto character = user.character();
                    
                    //draw portrait
                    auto portrait = RPZCharacter::getPortrait(character);
                    painter->drawPixmap(option.rect, portrait);

                    //draw gauges
                    //TODO

                    //draw bonus / malus
                    //TODO

                    //draw color indicator
                    painter->save();
                        QPen pen;
                        pen.setColor(user.color());
                        pen.setWidth(5);
                        painter->setPen(pen);
                        painter->drawRect(option.rect);
                    painter->restore();

                }
                break;

            }           

        }
        
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {

            auto out = QStyledItemDelegate::sizeHint(option, index);

            auto user = this->_getUser(index);
            switch(user.role()) {
                
                case RPZUser::Role::Observer:
                case RPZUser::Role::Host: {
                    return out;
                }
                break;
                
                case RPZUser::Role::Player: {
                    return defaultPortraitSize;
                }
                break;

            }

            return out;
        }
    private:
        static inline const QSize defaultPortraitSize = QSize(120, 160);

        const RPZUser _getUser(const QModelIndex &index) const {
            return RPZUser(index.data(Qt::UserRole).toHash());
        }
};