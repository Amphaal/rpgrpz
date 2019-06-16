#pragma once

#include "src/shared/models/RPZAtom.h"

#include <QGraphicsPathItem>

class AtomConverter {
    
    enum DataIndex { TemplateAtom = 222, BrushTransform = 555, IsTemporary = 666 };

    public:
        static void updateGraphicsItemFromAtom(QGraphicsItem* target, RPZAtom &blueprint, bool isTargetTemporary = false) {
            
            //bind a copy of the template to the item
            target->setData(TemplateAtom, RPZAtom(blueprint));
            target->setData(IsTemporary, isTargetTemporary);

            //update GI
            for(auto param : blueprint.legalEditedMetadata()) {

                auto val = blueprint.metadata(param);

                updateGraphicsItemFromMetadata(target, param, val);
            }

            //specific update
            if(auto casted = dynamic_cast<QGraphicsPathItem*>(target)) {
                
                if(blueprint.type() == AtomType::Drawing) {
                    auto pen = casted->pen();
                    pen.setColor(blueprint.owner().color());
                    casted->setPen(pen);
                }

            }
                    
        }

        static void updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomParameter &param,  QVariant &val) {
            
            if(!item) return;
            
            auto requiresTransform = _setParamToGraphicsItemFromAtom(param, item, val);

            if(requiresTransform) {
                if(auto cItem = dynamic_cast<QGraphicsPathItem*>(item)) {
                    _bulkTransformApply(cItem);
                }
            }
        };

        static RPZAtom graphicsToAtom(QGraphicsItem* blueprint) {
            
            //recover template
            auto atom = RPZAtom(blueprint->data(TemplateAtom).toHash());

            //get legal custom metadata
            auto templateMetadata = atom.legalEditedMetadata();
            templateMetadata.insert(AtomParameter::Position); //force position update

            //for each param to set to atom
            for(auto param : templateMetadata) {
                _setParamToAtomFromGraphicsItem(param, atom, blueprint);
            }
                        
            //finally, give it ID
            atom.shuffleId();

            return atom;
        }

    private:

        void static _bulkTransformApply(QGraphicsPathItem* itemBrushToUpdate) {
            
            auto transforms = itemBrushToUpdate->data(BrushTransform).toHash();
            
            //apply transforms to object
            QTransform toApply;
            for(auto i = transforms.begin(); i != transforms.end(); i++) {
                
                auto param = (AtomParameter)i.key().toInt();
                
                switch(param) {
                    case AssetScale: {
                        auto scaleRatio = i.value().toDouble();
                        toApply.scale(scaleRatio, scaleRatio);
                    }
                    break;

                    case AssetRotation: {
                        auto degrees = i.value().toInt();
                        toApply.rotate(degrees);
                    }
                    break;
                }

            }

            //apply transform
            auto brush = itemBrushToUpdate->brush();
            brush.setTransform(toApply);
            itemBrushToUpdate->setBrush(brush);
        }

        bool static _setParamToGraphicsItemFromAtom(const AtomParameter &param, QGraphicsItem* itemToUpdate, QVariant &val) {
            
            switch(param) {
                                
                    //on moving
                    case AtomParameter::Position: {
                        auto destPos = val.toPointF();
                        itemToUpdate->setPos(destPos);  
                    }
                    break;

                    // on locking change
                    case AtomParameter::Locked: {
                        auto locked = val.toBool();
                        auto flags = !locked ? MapViewItemsNotifier::defaultFlags() : 0;
                        itemToUpdate->setFlags(flags);
                    }
                    break;
                    
                    // on changing visibility
                    case AtomParameter::Hidden: {
                        auto hidden = val.toBool();
                        auto opacity = hidden ? .05 : 1;
                        itemToUpdate->setOpacity(opacity);
                    }
                    break;

                    //on rotation
                    case AtomParameter::Rotation: {
                        auto destRotation = val.toInt();
                        itemToUpdate->setRotation(destRotation);
                    }
                    break;

                    //on scaling
                    case AtomParameter::Scale: {
                        auto destScale = val.toDouble();
                        itemToUpdate->setScale(destScale);
                    }
                    break;

                    //on text size change
                    case AtomParameter::TextSize: {
                        if(auto cItem = dynamic_cast<QGraphicsTextItem*>(itemToUpdate)) {
                            auto newSize = val.toInt();
                            auto font = cItem->font();
                            font.setPointSize(newSize);
                            cItem->setFont(font);
                        }
                    }
                    break;

                    //on pen width change
                    case AtomParameter::PenWidth: {
                        if(auto cItem = dynamic_cast<QGraphicsPathItem*>(itemToUpdate)) {
                            auto newWidth = val.toInt();
                            auto pen = cItem->pen();
                            pen.setWidth(newWidth);
                            cItem->setPen(pen);
                        }
                    }
                    break;

                    //on text change
                    case AtomParameter::Text: {
                        if(auto cItem = dynamic_cast<QGraphicsTextItem*>(itemToUpdate)) {
                            auto newText = val.toString();
                            cItem->setPlainText(newText);
                        }
                    }
                    break;

                    //on layer change
                    case AtomParameter::Layer: {
                        
                        auto newLayer = val.toInt();

                        //always force temporary item on top of his actual set layer index
                        auto isTemporary = itemToUpdate->data(IsTemporary).toBool();
                        if(isTemporary) newLayer++;
 
                        itemToUpdate->setZValue(newLayer);

                    }
                    break;

                    //on asset rotation / scale, store metadata for all-in transform update in main method
                    case AtomParameter::AssetRotation: {
                    case AtomParameter::AssetScale: {
                        auto transforms = itemToUpdate->data(BrushTransform).toHash();
                        transforms[QString::number(param)] = val;
                        itemToUpdate->setData(BrushTransform, transforms);
                        return true;
                    }
                    break;
                }
            }

            //no transform to apply
            return false;
        }

        void static _setParamToAtomFromGraphicsItem(const AtomParameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint) {
            switch(param) {
                
                case Scale: {
                    atomToUpdate.setMetadata(param, blueprint->scale());
                }
                break;

                case Rotation: {
                    atomToUpdate.setMetadata(param, blueprint->rotation());
                }
                break;

                case TextSize: {
                    if(auto casted = dynamic_cast<QGraphicsTextItem*>(blueprint)) {
                        atomToUpdate.setMetadata(param, casted->font().pointSize());
                    }
                }
                break;

                case Layer: {

                    auto layer = blueprint->zValue();

                    //if is a temporary, reset layer to expected value
                    auto isTemporary = blueprint->data(IsTemporary).toBool();
                    if(isTemporary) layer--;

                    atomToUpdate.setMetadata(param, layer);     

                }
                break;

                case PenWidth: {
                    if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(blueprint)) {
                        atomToUpdate.setMetadata(param, pathItem->pen().width()); 
                    }
                }
                break;
                
                case Shape: {
                    if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(blueprint)) {
                        atomToUpdate.setShape(pathItem->path()); 
                    } else {
                        atomToUpdate.setShape(blueprint->boundingRect());
                    }
                }
                break;

                case Position: {
                    atomToUpdate.setMetadata(param, blueprint->pos());     
                }
                break;
                
                case AssetScale:
                case AssetRotation: {
                    auto transforms = blueprint->data(BrushTransform).toHash();
                    if(transforms.isEmpty()) return;

                    auto transform = transforms[QString::number(param)];
                    if(transform.isNull()) return;
                    
                    atomToUpdate.setMetadata(param, transform); 
                }
                break;
            }
        }   
};