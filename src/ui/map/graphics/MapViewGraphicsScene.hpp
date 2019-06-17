#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "MapViewGraphicsItem.hpp"
#include "MapViewItemsNotifier.hpp"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/AssetMetadata.hpp"

#include "src/helpers/AtomConverter.hpp"

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        void onItemChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override {
            emit sceneItemChanged(item, (int)flag);
        };

    public:
        MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

        QGraphicsItem* addToScene(RPZAtom &atom, AssetMetadata &assetMetadata, bool isTemporary = false) {
            
            QGraphicsItem* out;

            switch(atom.type()) {
                
                case AtomType::Object:
                    out = this->_addGenericImageBasedItem(atom, assetMetadata);
                break;
                
                case AtomType::Brush:
                    out = this->_addBrush(atom, assetMetadata);
                break;

                case AtomType::Drawing:
                    out = this->_addDrawing(atom);
                break;

                case AtomType::Text:
                    out = this->_addText(atom);
                break;

                default:
                    return nullptr;
                break;
            }

            //add atomType tracker
            AtomConverter::updateGraphicsItemFromAtom(out, atom, isTemporary);
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
            AtomConverter::updateGraphicsItemFromAtom(placeholder, atom);
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

        QGraphicsPathItem* _addBrush(RPZAtom &atom, AssetMetadata &assetMetadata) {
            
            //define a ped
            QPen pen;
            pen.setWidth(atom.penWidth());
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            pen.setColor(QColor(255, 255, 255, 0));

            //define a default shape for ghost items
            auto shape = atom.shape();
            shape.setFillRule(Qt::FillRule::WindingFill);

            //configure brush
            QBrush brush;
            auto fpath = assetMetadata.pathToAssetFile();
            brush.setTexture(QPixmap(fpath));
            
            //conditionnal binding
            QBrush giBrush;
            if(atom.brushType() == BrushType::RoundBrush) {
                pen.setBrush(brush);
            } else {
                giBrush = brush;
            }
            
            //create path
            auto newPath = new MapViewGraphicsPathItem(this, shape, pen, giBrush);
            
            return newPath;
        }

        QGraphicsPathItem* _addDrawing(RPZAtom &atom) {
            
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

            //create path
            auto newPath = new MapViewGraphicsPathItem(this, shape, pen);
            
            return newPath;
        }

        QGraphicsTextItem* _addText(RPZAtom &atom) {
            return new MapViewGraphicsTextItem(this, atom.text(), atom.textSize());
        }

};