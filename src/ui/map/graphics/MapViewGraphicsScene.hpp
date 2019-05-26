#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "MapViewGraphicsItem.hpp"
#include "MapViewItemsNotifier.hpp"

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int assetAlteration);

    private:
        void onItemChange(QGraphicsItem* item, RPZAsset::Alteration alteration) override {
            emit sceneItemChanged(item, alteration);
        };

    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) {}

        QGraphicsItem* addGenericImageBasedAsset(const QString &pathToImageFile, qreal opacity = 1, const QPointF &initialPos = QPointF()) {
    
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

            //add it to the scene
            this->addItem(item);

            //define position
            if(!initialPos.isNull()) item->setPos(initialPos);

            return item;
        }

        QGraphicsPathItem* addDrawing(const QPainterPath &path, const QPen &pen) {
            
            //add path
            auto newPath = new MapViewGraphicsPathItem(this, path, pen);
            this->addItem(newPath);
            
            return newPath;
        }


        MapViewGraphicsRectItem* addMissingAssetPH(QRectF &rect) {
    
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
            this->addItem(placeholder);

            return placeholder;
        }
};