#include "AtomConverter.h"

void AtomConverter::updateGraphicsItemFromAtom(QGraphicsItem* target, RPZAtom &blueprint, bool isTargetTemporary) {
    
    //bind a copy of the template to the item
    target->setData((int)AtomConverterDataIndex::TemplateAtom, RPZAtom(blueprint));
    target->setData((int)AtomConverterDataIndex::IsTemporary, isTargetTemporary);

    //refresh all legal if temporary
    auto paramsToUpdate = blueprint.legalParameters().toList();
    std::sort(paramsToUpdate.begin(), paramsToUpdate.end()); 

    //update GI
    for(auto param : paramsToUpdate) {
        auto val = blueprint.metadata(param);
        updateGraphicsItemFromMetadata(target, param, val);
    }

    //specific update on type 
    switch(blueprint.type()) {
        
        //on drawing...
        case AtomType::Drawing: {

            //update pen color to owner
            if(auto casted = dynamic_cast<MapViewGraphicsPathItem*>(target)) {      
                auto pen = casted->pen();
                pen.setColor(blueprint.owner().color());
                casted->setPen(pen);
            }

        }
        break;

        default:
            break;

    }
            
}

void AtomConverter::updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomParameter &param, QVariant &val) {
    
    if(!item) return;
    
    auto requiresTransform = _setParamToGraphicsItemFromAtom(param, item, val);
    if(requiresTransform) _bulkTransformApply(item);

};

RPZAtom AtomConverter::graphicsToAtom(QGraphicsItem* blueprint) {
    
    //recover template
    auto templateAtom = RPZAtom(blueprint->data((int)AtomConverterDataIndex::TemplateAtom).toHash());
    
    //update the 2 only parameters who might have changed from the template
    _setParamToAtomFromGraphicsItem(AtomParameter::Position, templateAtom, blueprint);
    _setParamToAtomFromGraphicsItem(AtomParameter::Shape, templateAtom, blueprint);
                
    //finally, give it ID for storage
    templateAtom.shuffleId();

    return templateAtom;
}


bool AtomConverter::_isTemporary(QGraphicsItem* item) {
    return item->data((int)AtomConverterDataIndex::IsTemporary).toBool();
}

void AtomConverter::_bulkTransformApply(QGraphicsItem* itemBrushToUpdate) {
    
    //cast to dest GI type
    auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemBrushToUpdate);
    if(!cItem) return;

    //extract transform instructions
    auto transforms = itemBrushToUpdate->data((int)AtomConverterDataIndex::BrushTransform).toHash();
    
    
    QTransform toApply;

    //apply transforms to instruction object
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

            default:
                break;
        }

    }

    //extract brush type
    auto type = (BrushType)itemBrushToUpdate->data((int)AtomConverterDataIndex::BrushDrawStyle).toInt();
    
    //apply to pen
    if(type == BrushType::RoundBrush) {
        auto pen = cItem->pen();
        auto brush = pen.brush();
        brush.setTransform(toApply);
        pen.setBrush(brush);
        cItem->setPen(pen);
    }

    //apply to GI brush
    else {
        auto brush = cItem->brush();
        brush.setTransform(toApply);
        cItem->setBrush(brush);
    }

}

bool AtomConverter::_setParamToGraphicsItemFromAtom(const AtomParameter &param, QGraphicsItem* itemToUpdate, QVariant &val) {
    
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
                auto flags = !locked ? MapViewItemsNotifier::defaultFlags() : QFlags<QGraphicsItem::GraphicsItemFlag>();
                itemToUpdate->setFlags(flags);
            }
            break;
            
            // on changing visibility
            case AtomParameter::Hidden: {
                if(!_isTemporary(itemToUpdate)) {
                    auto hidden = val.toBool();
                    auto opacity = hidden ? .05 : 1;
                    itemToUpdate->setOpacity(opacity);
                }
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
            case AtomParameter::BrushPenWidth:
            case AtomParameter::PenWidth: {
                if(auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemToUpdate)) {
                    auto newWidth = val.toInt();
                    auto pen = cItem->pen();
                    pen.setWidth(newWidth);
                    cItem->setPen(pen);
                }
            }
            break;

            case AtomParameter::BrushStyle: {
                if(auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemToUpdate)) {
                    
                    auto type = (BrushType)val.toInt();
                    itemToUpdate->setData((int)AtomConverterDataIndex::BrushDrawStyle, val);
                    
                    //use pen as brush
                    if(type == BrushType::RoundBrush) {
                        
                        //reset brush
                        cItem->setBrush(QBrush());
                        
                        //set pen
                        auto pen = cItem->pen();
                        pen.setBrush(cItem->sourceBrush());
                        cItem->setPen(pen);
                    } 
                    
                    //use item brush
                    else {

                        //reset pen
                        auto pen = cItem->pen();
                        pen.setBrush(QBrush());
                        cItem->setPen(pen);

                        //set brush
                        cItem->setBrush(cItem->sourceBrush());
                    }

                    //define default shape for temporary
                    if(_isTemporary(itemToUpdate)) {
                        
                        QPainterPath path;

                        if(type == BrushType::Stamp) {
                            QRectF rect(QPointF(0,0), cItem->sourceBrushSize());
                            path.addRect(rect);
                        } 
                        
                        else {
                            path.lineTo(.01,.01);
                        }

                        cItem->setPath(path);
                    } 
                    
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
                if(_isTemporary(itemToUpdate)) newLayer++;

                itemToUpdate->setZValue(newLayer);

            }
            break;
            
            //on asset rotation / scale, store metadata for all-in transform update in main method
            case AtomParameter::AssetRotation: {
            case AtomParameter::AssetScale: {
                auto transforms = itemToUpdate->data((int)AtomConverterDataIndex::BrushTransform).toHash();
                transforms[QString::number(param)] = val;
                itemToUpdate->setData((int)AtomConverterDataIndex::BrushTransform, transforms);
                return true;
            }
            break;

            default:
                break;
        }
    }

    //no transform to apply
    return false;
}

void AtomConverter::_setParamToAtomFromGraphicsItem(const AtomParameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint) {
    switch(param) {
        
        case AtomParameter::Scale: {
            atomToUpdate.setMetadata(param, blueprint->scale());
        }
        break;

        case AtomParameter::Rotation: {
            atomToUpdate.setMetadata(param, blueprint->rotation());
        }
        break;

        case AtomParameter::TextSize: {
            if(auto casted = dynamic_cast<QGraphicsTextItem*>(blueprint)) {
                atomToUpdate.setMetadata(param, casted->font().pointSize());
            }
        }
        break;

        case AtomParameter::Layer: {

            auto layer = blueprint->zValue();

            //if is a temporary, reset layer to expected value
            if(_isTemporary(blueprint)) layer--;

            atomToUpdate.setMetadata(param, layer);     

        }
        break;

        case AtomParameter::BrushStyle: {
            auto brushStyle = blueprint->data((int)AtomConverterDataIndex::BrushDrawStyle).toInt();
            atomToUpdate.setMetadata(param, brushStyle);
        }
        break;

        case AtomParameter::BrushPenWidth:
        case AtomParameter::PenWidth: {
            if(auto pathItem = dynamic_cast<MapViewGraphicsPathItem*>(blueprint)) {
                atomToUpdate.setMetadata(param, pathItem->pen().width()); 
            }
        }
        break;

        case AtomParameter::Shape: {
            if(auto pathItem = dynamic_cast<MapViewGraphicsPathItem*>(blueprint)) {
                atomToUpdate.setShape(pathItem->path()); 
            } else {
                atomToUpdate.setShape(blueprint->boundingRect());
            }
        }
        break;

        case AtomParameter::Position: {
            atomToUpdate.setMetadata(param, blueprint->pos());     
        }
        break;
        
        case AtomParameter::AssetScale:
        case AtomParameter::AssetRotation: {
            auto transforms = blueprint->data((int)AtomConverterDataIndex::BrushTransform).toHash();
            if(transforms.isEmpty()) return;

            auto transform = transforms[QString::number(param)];
            if(transform.isNull()) return;
            
            atomToUpdate.setMetadata(param, transform); 
        }
        break;

        default:
            break;
    }
}   
