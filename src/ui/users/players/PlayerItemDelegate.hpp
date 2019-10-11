#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QRect>

#include "src/shared/models/RPZUser.h"
#include "src/ui/sheets/components/GaugeWidget.hpp"

class PlayerItemDelegate : public QStyledItemDelegate {
    public:
        PlayerItemDelegate() { }

        static inline const QSize defaultPortraitSize = QSize(120, 160);
        static inline const int gaugeHeight = 14;
        static inline const int portraitFrameMargin = 2;
        static inline const int portraitFramePenSize = 2;
        static inline const int spaceBetweenGauges = 2;

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            
            QStyledItemDelegate::paint(painter, option, index);

            auto user = this->_getUser(index);
            auto character = user.character();
            auto notFocusedSelected = !this->_isFocusedSelected(option);
            
            // draw portrait
            this->_printPortrait(painter, option, character);
            
            // draw bonus / malus
            if(notFocusedSelected) this->_printStatusIndicators(painter, option, character);

            // draw gauges
            if(notFocusedSelected) this->_printGauges(painter, option, character);
            
            // draw color indicator
            if(notFocusedSelected) this->_printPortraitFrame(painter, option, user);

        }
        
    private:
        const RPZUser _getUser(const QModelIndex &index) const {
            return RPZUser(index.data(Qt::UserRole).toHash());
        }
        
        void _printStatusIndicators(QPainter *painter, const QStyleOptionViewItem &option, const RPZCharacter &character) const {
            
            auto startPoint = option.rect.topRight();
            startPoint += QPoint(0, defaultPortraitSize.height());
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

        void _printGauges(QPainter *painter, const QStyleOptionViewItem &option, const RPZCharacter &character) const {
            
            if(!character.favGaugesCount()) return;

            QRect gaugeRect;
            gaugeRect.setHeight(PlayerItemDelegate::gaugeHeight);
            gaugeRect.setWidth(
                option.rect.width() 
                - (portraitFrameMargin * 2) 
                - (portraitFramePenSize * 2)
                - 1
            );

            auto startPoint = option.rect.topLeft();
            startPoint = startPoint + QPoint(0, defaultPortraitSize.height());
            startPoint = startPoint + QPoint(PlayerItemDelegate::portraitFrameMargin + PlayerItemDelegate::portraitFramePenSize, 0);
            gaugeRect.moveTo(startPoint);

            painter->save();

                QPen pen;
                pen.setWidth(1);
                
                auto font = painter->font();
                font.setPixelSize(PlayerItemDelegate::gaugeHeight - 4);
                painter->setFont(font);

                QTextOption tOption(Qt::AlignVCenter | Qt::AlignRight);

                for(auto &gauge : character.gauges()) {
                    
                    //do not display if not visible
                    if(!gauge.isVisibleUnderPortrait()) continue;

                    auto color = gauge.color();

                    //draw outer gauge
                    pen.setColor("#111");
                    painter->setPen(pen);
                    painter->drawRoundedRect(gaugeRect, 1, 1);

                    //calculate gauge ratio
                    auto gVal = gauge.gaugeValue();
                    auto gMax = gauge.maxGaugeValue();
                    double gaugeRatio = 0;
                    if(gVal && gMax) {
                        gaugeRatio = (double)gVal / gMax;
                    }

                    auto innerGaugeRect = gaugeRect.marginsRemoved(QMargins(1, 1, 0, 0));
                    
                    //text indicator
                    pen.setColor("#000");
                    painter->setPen(pen);
                    
                    // auto textContent = QString("%1% ").arg(
                    //     QLocale::system().toString(gaugeRatio * 100, 'g', 3)
                    // );
                    auto textContent = QString("%1/%2 ").arg(gVal).arg(gMax);

                    painter->drawText(innerGaugeRect, textContent, tOption);
                
                    // if gauge ratio is positive, print inner gauge
                    if(gaugeRatio > 0) {
                        
                        //reduce innerGaugeRect
                        innerGaugeRect.setWidth(
                            gaugeRatio * innerGaugeRect.width()
                        );

                        //print gauge indicator
                        painter->setOpacity(.5);    
                            painter->fillRect(innerGaugeRect, color);
                        painter->setOpacity(1);  

                    }

                    //prepare for next
                    gaugeRect.translate(0, PlayerItemDelegate::gaugeHeight + PlayerItemDelegate::spaceBetweenGauges);

                }

            painter->restore();
    
        }

        void _printPortrait(QPainter *painter, const QStyleOptionViewItem &option, const RPZCharacter &character) const {

            painter->save();
                
                if(this->_isFocusedSelected(option)) {
                    painter->setOpacity(0.5);
                }
                
                auto portrait = RPZCharacter::getPortrait(character);
                QRect portraitRect(option.rect.topLeft(), PlayerItemDelegate::defaultPortraitSize);

                painter->drawPixmap(portraitRect, portrait);

            painter->restore();

        }

        void _printPortraitFrame(QPainter *painter, const QStyleOptionViewItem &option, const RPZUser &user) const {

            painter->save();

                QPen pen;
                pen.setColor(user.color());
                pen.setWidth(portraitFramePenSize);
                pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
                painter->setPen(pen);

                auto portraitRect = option.rect.marginsRemoved(
                    QMargins(portraitFrameMargin, portraitFrameMargin, portraitFrameMargin, portraitFrameMargin)
                );
                painter->drawRect(portraitRect);

            painter->restore();

        }

        bool _isFocusedSelected(const QStyleOptionViewItem &option) const {
            return option.state.testFlag(QStyle::State_Selected) && option.state.testFlag(QStyle::State_HasFocus);
        }
};