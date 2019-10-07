#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QRect>

#include "src/shared/models/RPZUser.h"
#include "src/ui/sheets/components/Gauge.hpp"



class UserItemDelegate : public QStyledItemDelegate {
    public:
        UserItemDelegate() { }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            
            auto user = this->_getUser(index);

            switch(user.role()) {
                
                case RPZUser::Role::Observer:
                case RPZUser::Role::Host: {

                }
                break;

                
                case RPZUser::Role::Player: {
                    auto character = user.character();
                    auto portrait = RPZCharacter::getPortrait(character);
                    painter->drawPixmap(option.rect, portrait);
                    
                    // qDebug() << option;
                    // QStyledItemDelegate::paint(painter, option, index);
                }
                break;

            }           

        }
        
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {

            auto user = this->_getUser(index);
            auto out = QStyledItemDelegate::sizeHint(option, index);

            switch(user.role()) {
                
                case RPZUser::Role::Observer:
                case RPZUser::Role::Host: {

                }
                break;
                
                case RPZUser::Role::Player: {
                    out.setHeight(
                        defaultPortraitSize.height()
                    );
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