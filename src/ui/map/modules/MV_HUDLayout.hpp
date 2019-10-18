#pragma once

#include <QGraphicsView>

#include "src/helpers/StringHelper.hpp"
#include "src/shared/hints/MapHint.h"

class MV_HUDLayout {
    public:
        MV_HUDLayout(QGraphicsView* view) : _view(view),
        _heavyLoadImage(QPixmap(":/icons/app_64.png")),
        _stdTileSize(AppContext::standardTileSize(view)) { }
    
    protected:
        void extractMapParametersForHUDLayout(const MapHint* hint) {
            this->_tileToMeterRatio = hint->tileToMeterRatio();
        }

        void setupHeavyLoadPlaceholder(int expectedItemsCount) {
            this->_heavyLoadExpectedCount = expectedItemsCount;
            this->_heavyLoadCurrentCount = 0;
        };

        void endHeavyLoadPlaceholder() {
            this->_heavyLoadExpectedCount = -1;
            this->_heavyLoadCurrentCount = -1;
            this->_heavyLoadColor = QColor();
        }

        void displayHeavyLoadPlaceholder() {
            this->_heavyLoadColor = RandomColor::getRandomColor();
            this->_heavyLoadExpectedCount = 0;
            this->_heavyLoadCurrentCount = 0;
        }

        void mayUpdateHeavyLoadPlaceholder(QPainter* painter) {
            this->_mayUpdateHeavyLoadPlaceholder(painter);
        }

        void incrementHeavyLoadPlaceholder() {
            this->_heavyLoadCurrentCount++;
            QPainter p(this->_view->viewport());
            this->_mayUpdateHeavyLoadPlaceholder(&p);
        }

        void mayUpdateHUD(QPainter* painter, const QRectF &rect) {

            if(this->_heavyLoadExpectedCount > -1) return;

            painter->save();

                auto viewportRect = this->_view->rect();
                auto currentScale = this->_view->transform().m11();

                this->_mayDrawGridIndic(painter, rect, currentScale);

                //ignore transformations
                QTransform t;
                painter->setTransform(t);

                this->_mayDrawZoomIndic(painter, viewportRect, currentScale);
                this->_mayDrawScaleIndic(painter, viewportRect, currentScale);

            painter->restore();

        }

        void drawBackgroundCheckerboard(QPainter *painter, const QRectF &rect) {
            
            painter->save();

                //no antialisation nor smothing
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, false);

                //set opacity
                painter->setOpacity(0.1);

                //pattern
                QBrush brush;
                brush.setStyle(Qt::Dense4Pattern);
                brush.setColor(Qt::GlobalColor::black);
                painter->setBrush(brush);

                //override pen
                painter->setPen(Qt::NoPen);

                //ignore transformations
                QTransform t;
                t.scale(10, 10);
                painter->setTransform(t);

                painter->drawRect(this->_view->rect());

            painter->restore();

        }

        void drawForeground() {

        }
    
    private:
        QGraphicsView* _view = nullptr;
        double _tileToMeterRatio = AppContext::DEFAULT_TILE_TO_METER_RATIO;
        QSizeF _stdTileSize;

        //background / foreground
        QPixmap _heavyLoadImage;
        QColor _heavyLoadColor;
        int _heavyLoadExpectedCount = -1;
        int _heavyLoadCurrentCount = -1;

        void _mayUpdateHeavyLoadPlaceholder(QPainter* painter) {
            
            if(this->_heavyLoadExpectedCount < 0) return;

            auto viewportRect = this->_view->rect();
            auto viewportCenter = viewportRect.center();

            painter->save();

                //hide
                painter->setBrush(QBrush("#EEE", Qt::SolidPattern));
                painter->setPen(Qt::NoPen);
                painter->setTransform(QTransform()); //ignore transformations
                painter->drawRect(viewportRect);
                
                //draw gauge
                if(this->_heavyLoadExpectedCount > 0) {
                    
                    //ext gauge
                    QRect extGauge(QPoint(0, 0), QSize(102, 12));
                    extGauge.moveCenter(viewportCenter);
                    painter->setBrush(QBrush("#DDD", Qt::SolidPattern));
                    painter->setPen(QPen(Qt::NoBrush, 0));
                    painter->drawRect(extGauge);

                    //draw inner
                    if(this->_heavyLoadCurrentCount > 0) {
                        
                        painter->setOpacity(.5);

                        //define inner gauge
                        auto innerGauge = extGauge.marginsRemoved(QMargins(1, 1, 1, 1));
                        auto ratio = (double)this->_heavyLoadCurrentCount / this->_heavyLoadExpectedCount;
                        auto newWidth = (int)(ratio * innerGauge.width());
                        innerGauge.setWidth(newWidth);

                        //draw it
                        painter->setBrush(QBrush(this->_heavyLoadColor, Qt::SolidPattern));
                        painter->setPen(Qt::NoPen);
                        painter->drawRect(innerGauge);

                        painter->setOpacity(1);

                    }

                }

                //draw pixmap
                auto pixmapRect = this->_heavyLoadImage.rect();
                auto alteredCenter = viewportCenter;
                alteredCenter.setY(
                    viewportCenter.y() - 6 - 32
                );
                pixmapRect.moveCenter(alteredCenter);
                painter->drawPixmap(pixmapRect, this->_heavyLoadImage);

            painter->restore();

        }

        void _mayDrawZoomIndic(QPainter* painter, const QRect &viewportRect, double currentScale) {
            
            if(!AppContext::settings()->scaleActive()) return;

            painter->save();

                //zoom indic
                auto templt = QStringLiteral(u"Zoom : %1x");
                templt = templt.arg(currentScale, 0, 0, 2, 0);

                //background
                painter->setOpacity(.75);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QBrush("#FFF", Qt::SolidPattern));
                auto bgRect = painter->boundingRect(viewportRect, templt, QTextOption(Qt::AlignTop | Qt::AlignRight));
                bgRect = bgRect.marginsAdded(QMargins(5, 5, 5, 2));
                bgRect.moveTopRight(viewportRect.topRight());
                painter->drawRoundedRect(bgRect, 2, 2);
                
                //text
                painter->setOpacity(1);
                painter->setPen(QPen(Qt::SolidPattern, 0));
                painter->setBrush(Qt::NoBrush);
                painter->drawText(bgRect, templt, QTextOption(Qt::AlignCenter));

            painter->restore();

        }

        void _mayDrawScaleIndic(QPainter* painter, const QRect &viewportRect, double currentScale) {
            
            if(!AppContext::settings()->scaleActive()) return;

            painter->save();

                //no antialisation nor smothing
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, false);

                auto tileWidth = (int)this->_stdTileSize.width();
                auto stops = 5;
                auto rulerSize = (int)(tileWidth * stops);
                auto elipseSize = rulerSize + 50;
                auto ratio = this->_tileToMeterRatio * (1 / currentScale);

                //cover
                painter->setOpacity(.5);
                painter->setPen(Qt::NoPen);
                painter->setBrush(QBrush("#FFF", Qt::SolidPattern));
                painter->drawEllipse(viewportRect.bottomLeft(), elipseSize, elipseSize);
                painter->setOpacity(1);

                auto scalePos = viewportRect.bottomLeft();
                scalePos.setX(scalePos.x() + 15);
                scalePos.setY(scalePos.y() - 20);

                auto scaleDestX = scalePos;
                scaleDestX.setX(scalePos.x() + rulerSize);

                auto scaleDestY = scalePos;
                scaleDestY.setY(scalePos.y() - rulerSize);

                painter->setPen(QPen(Qt::SolidPattern, 0));
                painter->setBrush(Qt::NoBrush);
                painter->drawLine(scalePos, scaleDestX);
                painter->drawLine(scalePos, scaleDestY);

                //zero
                auto zeroRect = painter->boundingRect(viewportRect, QStringLiteral(u"0"));
                zeroRect.moveCenter(scalePos);
                zeroRect.moveTop(scalePos.y());
                painter->drawText(zeroRect, QStringLiteral(u"0"));

                for(auto stop = 1; stop <= stops; stop++) {
                    
                    auto pos = scalePos;
                    pos.setX(
                        pos.x() + (stop * tileWidth)
                    );

                    auto txtStr = StringHelper::fromMeters(ratio * stop);
                    auto textRect = painter->boundingRect(viewportRect, txtStr);
                    textRect.moveCenter(pos);
                    textRect.moveTop(pos.y());
                    painter->drawText(textRect, txtStr);

                    auto tickDest = pos;
                    tickDest.setY(
                        tickDest.y() - 5
                    );
                    painter->drawLine(pos, tickDest);
                    
                }



            painter->restore();
        }

        void _mayDrawGridIndic(QPainter* painter, const QRectF &rect, double currentScale) {
            
            if(!AppContext::settings()->gridActive()) return;
            if(currentScale < .1) return; //prevent if scale is too far

            painter->save();

                QPen pen(Qt::SolidPattern, 0);
                pen.setColor("#222");
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
                
                auto sceneRect = this->_view->sceneRect();
                auto center = sceneRect.center();

                auto sceneBottom = sceneRect.bottom();
                auto sceneRight = sceneRect.right();
                auto centerX = center.x();
                auto centerY = center.y();
                auto tileWidth = this->_stdTileSize.width();
                auto tileHeight = this->_stdTileSize.height();

                auto numberOfLinesX = (int)(centerX / tileWidth);
                auto numberOfLinesY = (int)(centerY / tileHeight);

                //origin lines
                auto horizontalOriginLine = QLineF(centerX, 0, centerX, sceneBottom);
                auto verticalOriginLine = QLineF(0, centerY, sceneRight, centerY);
                painter->drawLine(horizontalOriginLine); //x
                painter->drawLine(verticalOriginLine); //y

                //horizontal
                for(auto x = 1; x <= numberOfLinesX; x++) {
                    auto step = x * tileWidth;
                    auto centerXMore = centerX + step;
                    auto centerXLess = centerX - step;
                    painter->drawLine(QLineF(centerXMore, 0, centerXMore, sceneBottom));
                    painter->drawLine(QLineF(centerXLess, 0, centerXLess, sceneBottom));
                }

                //vertical
                for(int y = 1; y <= numberOfLinesY; y++) {
                    auto step = y * tileHeight;
                    auto centerYMore = centerY + step;
                    auto centerYLess = centerY - step;
                    painter->drawLine(QLineF(0, centerYMore, sceneRight, centerYMore));
                    painter->drawLine(QLineF(0, centerYLess, sceneRight, centerYLess));
                }

            painter->restore();

        }

};