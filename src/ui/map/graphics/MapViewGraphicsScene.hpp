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
            defaultColor(fallbackColor) { }; 

};

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        void onItemChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override {
            emit sceneItemChanged(item, (int)flag);
        };

        void _updateGraphicsItemFromAtom(QGraphicsItem* target, RPZAtom &blueprint) {
            target->setZValue(blueprint.layer());
            target->setPos(blueprint.pos());
            target->setScale(blueprint.scale());
            target->setRotation(blueprint.rotation());
            target->setData(1, RPZAtom(blueprint));
        }


    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

        static RPZAtom itemToAtom(QGraphicsItem* blueprint) {
            
            //recover template
            auto atom = RPZAtom(blueprint->data(1).toHash());

            //update template values from current gi properties
            atom.setPos(blueprint->scenePos());
            atom.setShape(blueprint->boundingRect());

            return atom;
        }

        QGraphicsItem* addToScene(RPZAtom &atom, MVPayload &aditionnalArgs) {
            
            QGraphicsItem* out;

            switch(atom.type()) {
                case AtomType::Object:
                    out = this->_addGenericImageBasedItem(atom, aditionnalArgs);
                break;
                
                case AtomType::Drawing:
                    out = this->_addDrawing(atom, aditionnalArgs);
                break;

                case AtomType::Text:
                    out = this->_addText(atom);
                break;

                default:
                    return nullptr;
                break;
            }

            //add atomType tracker
            this->_updateGraphicsItemFromAtom(out, atom);
            this->addItem(out);
            return out;
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
            this->_updateGraphicsItemFromAtom(placeholder, atom);
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

            return item;
        }

        QGraphicsPathItem* _addDrawing(RPZAtom &atom, MVPayload &aditionnalArgs) {
            
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

            //define a default shape for ghost items
            auto shape = atom.shape();
            if(!shape.elementCount()) {
                shape.lineTo(QPoint(.1,.1));
            }

            //create path
            auto newPath = new MapViewGraphicsPathItem(this, shape, pen);
            
            return newPath;
        }

        QGraphicsTextItem* _addText(RPZAtom &atom) {
            return new MapViewGraphicsTextItem(this, atom.text(), atom.penWidth());
        }

};