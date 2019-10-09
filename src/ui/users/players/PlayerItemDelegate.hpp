#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QRect>

#include "src/shared/models/RPZUser.h"
#include "src/ui/sheets/components/Gauge.hpp"

class PlayerItemDelegate : public QStyledItemDelegate {
    public:
        PlayerItemDelegate() { }

        static inline const QSize defaultPortraitSize = QSize(120, 160);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            
            QStyledItemDelegate::paint(painter, option, index);

            auto user = this->_getUser(index);
            auto character = user.character();
            auto notFocusedSelected = !this->_isFocusedSelected(option);
            
            // draw portrait
            this->_printPortrait(painter, option, character);
            
            // draw color indicator
            if(notFocusedSelected) this->_printPortraitFrame(painter, option, user);

            // draw bonus / malus
            if(notFocusedSelected) this->_printStatusIndicators(painter, option, character);

            // draw gauges
            // TODO

        }
        
    private:
        const RPZUser _getUser(const QModelIndex &index) const {
            return RPZUser(index.data(Qt::UserRole).toHash());
        }
        
        void _printStatusIndicators(QPainter *painter, const QStyleOptionViewItem &option, const RPZCharacter &character) const {
            
            auto startPoint = option.rect.bottomRight();
            startPoint = startPoint - QPoint(6, 6) - QPoint(12, 16);

            if(!character.malus().isEmpty()) {
                painter->drawPixmap(startPoint, 
                    QPixmap(":/icons/app/other/malus.png")
                );
                startPoint = startPoint - QPoint(14, 0);
            }
            
            if(!character.bonus().isEmpty()) {
                painter->drawPixmap(startPoint, 
                    QPixmap(":/icons/app/other/bonus.png")
                );
            }

        }

        void _printPortrait(QPainter *painter, const QStyleOptionViewItem &option, const RPZCharacter &character) const {

            painter->save();
                
                if(this->_isFocusedSelected(option)) {
                    painter->setOpacity(0.5);
                }
                
                auto portrait = RPZCharacter::getPortrait(character);
                painter->drawPixmap(option.rect, portrait);

            painter->restore();

        }

        void _printPortraitFrame(QPainter *painter, const QStyleOptionViewItem &option, const RPZUser &user) const {

            painter->save();

                QPen pen;
                pen.setColor(user.color());
                pen.setWidth(2);
                pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
                painter->setPen(pen);

                auto portraitRect = option.rect.marginsRemoved(QMargins(2, 2, 2, 2));
                painter->drawRect(portraitRect);

            painter->restore();

        }

        bool _isFocusedSelected(const QStyleOptionViewItem &option) const {
            return option.state.testFlag(QStyle::State_Selected) && option.state.testFlag(QStyle::State_HasFocus);
        }
};