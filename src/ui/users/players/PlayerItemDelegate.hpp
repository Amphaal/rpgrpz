#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QRect>

#include "src/shared/models/RPZUser.h"
#include "src/ui/sheets/components/GaugeWidget.hpp"

class PlayerItemDelegate : public QStyledItemDelegate {
    public:

        static inline const QSize DEFAULT_PORTRAIT_SIZE = QSize(120, 160);

        PlayerItemDelegate() { }

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
            if(notFocusedSelected) this->_printColorIndicator(painter, option, user);

        }

        static QSize sizeHint(const QModelIndex &index) {
            
            auto size = DEFAULT_PORTRAIT_SIZE;

            auto user = _getUser(index);
            auto character = user.character();

            if(auto favGaugesCount = character.favGaugesCount()) {
                
                auto moreHeight = (GAUGE_HEIGHT * favGaugesCount) + (favGaugesCount + 1) * SPACE_BETWEEN_GAUGES;

                size.setHeight(
                    size.height() 
                    + moreHeight
                );

            }

            return size;

        }

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            return sizeHint(index);
        }

    private:
        static inline const int GAUGE_HEIGHT = 14;
        static inline const int GAUGE_SIDE_MARGIN = 1;
        static inline const int SPACE_BETWEEN_GAUGES = 2;

        static const RPZUser _getUser(const QModelIndex &index) {
            return RPZUser(index.data(Qt::UserRole).toHash());
        }
        
        void _printStatusIndicators(QPainter *painter, const QStyleOptionViewItem &option, const RPZCharacter &character) const {
            
            auto startPoint = option.rect.topRight();
            startPoint += QPoint(0, DEFAULT_PORTRAIT_SIZE.height());
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
            
            //define pen
            QPen pen;
            pen.setWidth(1);

            //define text options
            QTextOption tOption(Qt::AlignVCenter | Qt::AlignRight);
            
            //define font
            auto font = painter->font();
            font.setPixelSize(GAUGE_HEIGHT - 4);

            //define gauge rect
            auto gaugeRect = this->_firstGaugeGeometry(option);

            painter->save();

                //apply font
                painter->setFont(font);

                //for each gauge...
                for(auto &gauge : character.gauges()) {
                    
                    //do not display if not visible
                    if(!gauge.isVisibleUnderPortrait()) continue;

                    //draw outer gauge
                    pen.setColor("#111");
                    painter->setPen(pen);
                    painter->drawRoundedRect(gaugeRect, 1, 1);

                    //extract current-max
                    auto gVal = gauge.gaugeValue();
                    auto gMax = gauge.maxGaugeValue();

                    //draw gauge values as text
                    pen.setColor("#000");
                    painter->setPen(pen);
                    auto textContent = QStringLiteral(u"%1/%2 ").arg(gVal).arg(gMax);
                    painter->drawText(gaugeRect, textContent, tOption);
                
                    //calculate gauge ratio
                    double gaugeRatio = 0;
                    if(gVal && gMax) {
                        gaugeRatio = (double)gVal / gMax;
                    }

                    // if gauge ratio is positive, print inner gauge
                    if(gaugeRatio > 0) {
                        
                        auto innerGaugeRect = gaugeRect;

                        //reduce innerGaugeRect
                        innerGaugeRect.setWidth(
                            (int)(gaugeRatio * innerGaugeRect.width())
                        );

                        //print gauge indicator
                        painter->setOpacity(.5);
                            painter->fillRect(innerGaugeRect, gauge.color());
                        painter->setOpacity(1);  

                    }

                    //prepare for next
                    gaugeRect.translate(0, GAUGE_HEIGHT + SPACE_BETWEEN_GAUGES);

                }

            painter->restore();
    
        }

        void _printPortrait(QPainter *painter, const QStyleOptionViewItem &option, const RPZCharacter &character) const {

            painter->save();
                
                if(this->_isFocusedSelected(option)) {
                    painter->setOpacity(0.5);
                }
                
                auto portrait = RPZCharacter::getPortrait(character);
                QRect portraitRect(option.rect.topLeft(), DEFAULT_PORTRAIT_SIZE);

                painter->drawPixmap(portraitRect, portrait);

            painter->restore();

        }

        void _printColorIndicator(QPainter *painter, const QStyleOptionViewItem &option, const RPZUser &user) const {

            painter->save();

                //pen
                QPen pen;
                pen.setWidth(0);
                pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
                painter->setPen(pen);

                //brush
                QBrush brush;
                brush.setStyle(Qt::BrushStyle::SolidPattern);
                brush.setColor(user.color());
                painter->setBrush(brush);
                
                //rect
                QRect indicator(QPoint(), QSize(8, 8));
                indicator.moveTopRight(option.rect.topRight() + QPoint(-4, 3));

                //draw
                painter->drawRect(indicator);

            painter->restore();

        }

        bool _isFocusedSelected(const QStyleOptionViewItem &option) const {
            return option.state.testFlag(QStyle::State_Selected) && option.state.testFlag(QStyle::State_HasFocus);
        }

        const QRect _firstGaugeGeometry(const QStyleOptionViewItem &option) const {
            
            //where to begin print
            auto startPoint = option.rect.topLeft();
            startPoint = startPoint + QPoint(
                GAUGE_SIDE_MARGIN, //left margin
                DEFAULT_PORTRAIT_SIZE.height() + SPACE_BETWEEN_GAUGES
            );

            //size preset of a single gauge
            QRect singleGaugePreset(
                startPoint, 
                QSize(
                    option.rect.width() - (GAUGE_SIDE_MARGIN + 2), //right margin
                    GAUGE_HEIGHT
                )
            );

            return singleGaugePreset;

        }
};