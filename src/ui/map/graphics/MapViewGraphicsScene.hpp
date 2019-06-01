#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "MapViewGraphicsItem.hpp"
#include "MapViewItemsNotifier.hpp"

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        void onItemChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override {
            emit sceneItemChanged(item, flag);
        };

    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) {}

        QGraphicsItem* addGenericImageBasedItem(const QString &pathToImageFile, int layer = 0, qreal opacity = 1, const QPointF &pos = QPointF()) {
    
            //get file infos
            QFileInfo pathInfo(pathToImageFile);
            
            //define graphicsitem
            QGraphicsItem* item = nullptr;
            if(pathInfo.suffix() == "svg") {
                item = new MapViewGraphicsSvgItem(this, pathToImageFile);
            } 
            else {
                item = new MapViewGraphicsPixmapItem(this, pathToImageFile);
            };

            //define transparency as it is a dummy
            item->setOpacity(opacity);

            //define layer
            item->setZValue(layer);

            //define position
            if(!pos.isNull()) {
                item->setPos(pos);
            }

            //add it to the scene
            this->addItem(item);

            return item;
        }

        QGraphicsPathItem* addDrawing(const QPainterPath &translatedPath, const QPen &pen, const QPointF &pos, int zIndex) {

            //create path gi, set to pos
            auto newPath = new MapViewGraphicsPathItem(this, translatedPath, pen);
            newPath->setPos(pos);
            newPath->setZValue(zIndex);
            
            //add to scene
            this->addItem(newPath);
            return newPath;
        }

        QGraphicsTextItem* addText(const QString &text, int textSize, QPointF &pos, int zIndex) {

            auto newText = new MapViewGraphicsTextItem(this, text, textSize);
            newText->setPos(pos);
            newText->setZValue(zIndex);

            //add to scene
            this->addItem(newText);
            return newText;
        }


        QGraphicsRectItem* addMissingAssetPH(QRectF &rect, QPointF &pos, int zIndex) {
    
            //pen to draw the rect with
            QPen pen;
            pen.setStyle(Qt::DashLine);
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setColor(Qt::GlobalColor::red);
            pen.setWidth(0);

            //background brush
            QBrush brush(QColor(255, 0, 0, 128));

            //add to scene
            auto placeholder = new MapViewGraphicsRectItem(this, rect, pen, brush);
            placeholder->setPos(pos);
            placeholder->setZValue(zIndex);
            this->addItem(placeholder);

            return placeholder;
        }
};