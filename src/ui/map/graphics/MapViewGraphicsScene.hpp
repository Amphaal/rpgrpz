#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "MapViewGraphicsItem.hpp"
#include "MapViewItemsNotifier.hpp"

#include "src/shared/models/entities/RPZAtom.h"

class MVPayload : public QVariantHash {
    
    public:
        QString pathToImageFile;
        QColor defaultColor; 
        
        MVPayload() {};
        MVPayload(const QColor &fallbackColor, const QString &pathToAsset = QString()) : 
            pathToImageFile(pathToAsset), 
            defaultColor(fallbackColor) { 

        }; 

};

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        void onItemChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override {
            emit sceneItemChanged(item, (int)flag);
        };

        void _bindDefaultMetadataToGraphicsItem(QGraphicsItem* item, RPZAtom &atom) {
            item->setZValue(atom.layer());
            item->setPos(atom.pos());
            item->setScale(atom.scale());
            item->setRotation(atom.rotation());
        }


    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

        QGraphicsItem* addToScene(RPZAtom &atom, MVPayload &aditionnalArgs = MVPayload()) {
            
            switch(atom.type()) {
                case AtomType::Object:
                    return this->_addGenericImageBasedItem(atom, aditionnalArgs);
                break;
                
                case AtomType::Drawing:
                    return this->_addDrawing(atom, aditionnalArgs);
                break;

                case AtomType::Text:
                    return this->_addText(atom);
                break;
            }

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

    protected:
        QGraphicsItem* _addGenericImageBasedItem(RPZAtom &atom, MVPayload &aditionnalArgs) {
    
            //get file infos
            auto pathToImageFile = aditionnalArgs.pathToImageFile;
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

        QGraphicsPathItem* _addDrawing(RPZAtom &atom, MVPayload &aditionnalArgs)) {
            
            //define a ped
            QPen pen;

                //if no owner set, assume it is self
                auto owner = atom.owner();
                if(owner.isEmpty()) {
                    pen.setColor(aditionnalArgs.defaultColor);
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

        QGraphicsTextItem* _addText(RPZAtom &atom) {

            auto newText = new MapViewGraphicsTextItem(this, atom.text(), atom.penWidth());
            
            //bind default
            this->_bindDefaultMetadataToGraphicsItem(newText, atom);

            //add to scene
            this->addItem(newText);

            return newText;
        }

};