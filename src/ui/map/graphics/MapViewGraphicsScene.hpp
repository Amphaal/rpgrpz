#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "MapViewGraphicsItem.hpp"
#include "MapViewItemsNotifier.hpp"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/AssetMetadata.hpp"


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
                font.setPointSize(blueprint.textSize());
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
            atom.setMetadata(
                RPZAtom::Parameters::Layer,
                atom.layer() - 1
            );
            
            //update template values from current gi properties
            atom.setMetadata(
                RPZAtom::Parameters::Position,
                blueprint->scenePos()
            );
            
            switch(atom.type()) {
                
                case AtomType::Text: {
                    auto casted = dynamic_cast<QGraphicsTextItem*>(blueprint);
                    atom.setShape(blueprint->boundingRect());
                    atom.setMetadata(RPZAtom::Parameters::TextSize, casted->font().pointSize());
                    atom.setMetadata(RPZAtom::Parameters::Text, casted->toPlainText());
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

        QGraphicsItem* addToScene(RPZAtom &atom, AssetMetadata &assetMetadata, bool isTemporary = false) {
            
            QGraphicsItem* out;

            switch(atom.type()) {
                
                case AtomType::Object:
                    out = this->_addGenericImageBasedItem(atom, assetMetadata);
                break;
                
                case AtomType::Brush:
                case AtomType::Drawing:
                    out = this->_addDrawing(atom, assetMetadata);
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

            //prevent notifications on move to kick in since the graphics item is not really in the scene
            if(isTemporary) {
                auto notifier = dynamic_cast<MapViewItemsNotifier*>(out);
                notifier->disableNotifications();
            }          

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
        QGraphicsItem* _addGenericImageBasedItem(RPZAtom &atom, AssetMetadata &assetMetadata) {
    
            //get file infos
            auto pathToImageFile = assetMetadata.pathToAssetFile();
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

        QGraphicsPathItem* _addDrawing(RPZAtom &atom, AssetMetadata &assetMetadata) {
            
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
            auto fpath = assetMetadata.pathToAssetFile();
            if(!fpath.isEmpty()) {
                brush.setTexture(QPixmap(fpath));
            } else {
                pen.setColor(atom.owner().color());
            }

            //create path
            auto newPath = new MapViewGraphicsPathItem(this, shape, pen, brush);
            
            return newPath;
        }

        QGraphicsTextItem* _addText(RPZAtom &atom) {
            return new MapViewGraphicsTextItem(this, atom.text(), atom.textSize());
        }

};