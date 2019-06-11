#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "MapViewGraphicsItem.hpp"
#include "MapViewItemsNotifier.hpp"

#include "src/shared/models/RPZAtom.h"

class MVPayload : public QVariantHash {
    
    public:
        QString pathToImageFile;
        QColor defaultColor; 
        
        MVPayload() {};
        MVPayload(const QString &pathToAsset = QString()) : 
            pathToImageFile(pathToAsset) { }; 

};

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        enum DataIndex { TemplateAtom = 222 };

        void onItemChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override {
            emit sceneItemChanged(item, (int)flag);
        };

    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

        static void updateGraphicsItemFromAtom(QGraphicsItem* target, RPZAtom &blueprint) {

            target->setZValue(blueprint.layer());
            target->setPos(blueprint.pos());
            target->setRotation(blueprint.rotation());
            target->setScale(blueprint.scale());

            target->setData(TemplateAtom, RPZAtom(blueprint));

            if(auto casted = dynamic_cast<QGraphicsTextItem*>(target)) {
                auto font = casted->font();
                font.setPointSize(blueprint.penWidth());
                casted->setFont(font);
            }

            else if(auto casted = dynamic_cast<QGraphicsPathItem*>(target)) {
                auto pen = casted->pen();
                pen.setWidth(blueprint.penWidth());
                pen.setColor(blueprint.owner().color());
                casted->setPen(pen);
            }
                    
        }

        static RPZAtom itemToAtom(QGraphicsItem* blueprint) {
            
            //recover template
            auto atom = RPZAtom(blueprint->data(TemplateAtom).toHash());
            atom.shuffleId();

            //remove -1 layer to the actual wanted layer
            atom.setLayer(
                atom.layer() - 1
            );
            
            //update template values from current gi properties
            atom.setPos(blueprint->scenePos());
            
            switch(atom.type()) {
                
                case AtomType::Text: {
                    auto casted = dynamic_cast<QGraphicsTextItem*>(blueprint);
                    atom.setShape(blueprint->boundingRect());
                    atom.setPenWidth(casted->font().pointSize());
                    atom.setText(casted->toPlainText());
                }
                break;

                case AtomType::Drawing: {
                    auto casted = dynamic_cast<QGraphicsPathItem*>(blueprint);
                    atom.setShape(casted->path()); 
                }
                break;

                default: {
                    atom.setShape(blueprint->boundingRect());
                }
                break;
            }

            return atom;
        }

        QGraphicsItem* addToScene(RPZAtom &atom, MVPayload &aditionnalArgs) {
            
            QGraphicsItem* out;

            switch(atom.type()) {
                
                case AtomType::Object:
                    out = this->_addGenericImageBasedItem(atom, aditionnalArgs);
                break;
                
                case AtomType::Brush:
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
            this->updateGraphicsItemFromAtom(out, atom);
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
            this->updateGraphicsItemFromAtom(placeholder, atom);
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
            pen.setWidth(atom.penWidth());
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);

            //define a default shape for ghost items
            auto shape = atom.shape();
            if(!shape.elementCount()) {
                shape.lineTo(.01,.01);
            }

            //add brush
            QBrush brush;
            if(!aditionnalArgs.pathToImageFile.isEmpty()) {
                brush.setTexture(QPixmap(aditionnalArgs.pathToImageFile));
            } else {
                pen.setColor(atom.owner().color());
            }

            //create path
            auto newPath = new MapViewGraphicsPathItem(this, shape, pen, brush);
            
            return newPath;
        }

        QGraphicsTextItem* _addText(RPZAtom &atom) {
            return new MapViewGraphicsTextItem(this, atom.text(), atom.penWidth());
        }

};