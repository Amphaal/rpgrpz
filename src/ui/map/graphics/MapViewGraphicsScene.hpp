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

        void _bindDefaultMetadataToGraphicsItem(QGraphicsItem* item, RPZAtom &atom) {
            item->setZValue(atom.layer());
            item->setPos(atom.pos());
            item->setScale(atom.scale());
            item->setRotation(atom.rotation());
        }


    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

        ///
        ///
        ///

        QGraphicsItem* addGenericImageBasedItem(const QString &pathToImageFile, RPZAtom &atom) {
    
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
            this->_bindDefaultMetadataToGraphicsItem(item, atom);
            
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

                //set pen
                pen.setWidth(atom.penWidth());
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);

            //create path gi, set to pos
            auto newPath = new MapViewGraphicsPathItem(this, atom.shape(), pen);
            
            //bind default
            this->_bindDefaultMetadataToGraphicsItem(newPath, atom);

            //add to scene
            this->addItem(newPath);

            return newPath;
        }

        QGraphicsTextItem* addText(RPZAtom &atom) {

            auto newText = new MapViewGraphicsTextItem(this, atom.text(), atom.penWidth());
            
            //bind default
            this->_bindDefaultMetadataToGraphicsItem(newText, atom);

            //add to scene
            this->addItem(newText);

            return newText;
        }


        QGraphicsRectItem* addMissingAssetPH(RPZAtom &atom) {
    
            //pen to draw the rect with
            QPen pen;
            pen.setStyle(Qt::DashLine);
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setColor(Qt::GlobalColor::red);
            pen.setWidth(0);

            //background brush
            QBrush brush(QColor(255, 0, 0, 128));

            //add to scene
            auto placeholder = new MapViewGraphicsRectItem(this, atom.shape().boundingRect(), pen, brush);
            
            //bind default
            this->_bindDefaultMetadataToGraphicsItem(placeholder, atom);

            this->addItem(placeholder);

            return placeholder;
        }
};