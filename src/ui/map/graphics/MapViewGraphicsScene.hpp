#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "MapViewGraphicsItem.hpp"
#include "MapViewItemsNotifier.hpp"

#include "src/shared/models/entities/RPZAtomMetadata.hpp"

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        void onItemChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override {
            emit sceneItemChanged(item, flag);
        };

        void _bindDefaultMetadataToGraphicsItem(QGraphicsItem* item, RPZAtomMetadata &metadata) {
            item->setZValue(metadata.layer());
            item->setPos(metadata.pos());
            item->setScale(metadata.scale());
            item->setRotation(metadata.rotation());
        }


    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

        ///
        ///
        ///

        QGraphicsItem* addGenericImageBasedItem(const QString &pathToImageFile, RPZAtomMetadata &metadata) {
    
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

            //bind default
            this->_bindDefaultMetadataToGraphicsItem(item, metadata);
            
            //add it to the scene
            this->addItem(item);

            return item;
        }

        QGraphicsPathItem* addDrawing(RPZAtom &atom, const QColor &fallbackColor = QColor()) {
            
            //define a ped
            QPen pen;

                //if no owner set, assume it is self
                auto owner = atom.owner();
                if(owner.isEmpty()) {
                    pen.setColor(fallbackColor);
                } else {
                    pen.setColor(owner.color());
                }

                //set width
                auto metadata = atom.metadata();

                pen.setWidth(metadata.penWidth());
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);

            //create path gi, set to pos
            auto newPath = new MapViewGraphicsPathItem(this, metadata.shape(), pen);
            
            //bind default
            this->_bindDefaultMetadataToGraphicsItem(newPath, metadata);

            //add to scene
            this->addItem(newPath);

            return newPath;
        }

        QGraphicsTextItem* addText(RPZAtomMetadata &metadata) {

            auto newText = new MapViewGraphicsTextItem(this, metadata.text(), metadata.penWidth());
            
            //bind default
            this->_bindDefaultMetadataToGraphicsItem(newText, metadata);

            //add to scene
            this->addItem(newText);

            return newText;
        }


        QGraphicsRectItem* addMissingAssetPH(RPZAtomMetadata &metadata) {
    
            //pen to draw the rect with
            QPen pen;
            pen.setStyle(Qt::DashLine);
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setColor(Qt::GlobalColor::red);
            pen.setWidth(0);

            //background brush
            QBrush brush(QColor(255, 0, 0, 128));

            //add to scene
            auto placeholder = new MapViewGraphicsRectItem(this, metadata.shape().boundingRect(), pen, brush);
            
            //bind default
            this->_bindDefaultMetadataToGraphicsItem(placeholder, metadata);

            this->addItem(placeholder);

            return placeholder;
        }
};