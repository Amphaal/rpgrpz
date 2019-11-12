#include "AtomConverter.h"

QVariantHash AtomConverter::brushTransform(QGraphicsItem *item) {
    return item->data((int)AtomConverter::DataIndex::BrushTransform).toHash();
}
void AtomConverter::setBrushTransform(QGraphicsItem *item, const QVariantHash &transforms) {
    item->setData((int)AtomConverter::DataIndex::BrushTransform, transforms);
}

bool AtomConverter::isTemporary(QGraphicsItem* item) {
    return item->data((int)AtomConverter::DataIndex::IsTemporary).toBool();
}
void AtomConverter::setIsTemporary(QGraphicsItem* item, bool isTemporary) {
    item->setData((int)AtomConverter::DataIndex::IsTemporary, isTemporary);
}

RPZAtom::BrushType AtomConverter::brushDrawStyle(QGraphicsItem* item) {
    return (RPZAtom::BrushType)item->data((int)AtomConverter::DataIndex::BrushDrawStyle).toInt();
}
void AtomConverter::setBrushDrawStyle(QGraphicsItem* item, const RPZAtom::BrushType &style) {
    item->setData((int)AtomConverter::DataIndex::BrushDrawStyle, (int)style);
}


void AtomConverter::updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom::Updates &updates) {
    //update GI
    for(auto i = updates.begin(); i != updates.end(); i++) {
        _updateGraphicsItemFromMetadata(item, i.key(), i.value());
    }
}

RPZAtom AtomConverter::graphicsToAtom(QGraphicsItem* blueprint, RPZAtom templateCopy) {
    
    //update the 3 only parameters who might have changed from the template
    _setParamToAtomFromGraphicsItem(RPZAtom::Parameter::Position, templateCopy, blueprint);
    _setParamToAtomFromGraphicsItem(RPZAtom::Parameter::Shape, templateCopy, blueprint);
                
    //finally, give it ID for storage
    templateCopy.shuffleId();

    return templateCopy;
}

void AtomConverter::updateGraphicsItemFromAtom(QGraphicsItem* target, const RPZAtom &blueprint, bool isTargetTemporary) {
    
    //set movable as default
    target->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, RPZClient::isHostAble());

    auto i = target->toGraphicsObject();

    //bind a copy of the template to the item
    setIsTemporary(target, isTargetTemporary);

    //update
    _updateGraphicsItemFromMetadata(target, blueprint);

    //if interactive, force ZIndex to max
    if(blueprint.category() == RPZAtom::Category::Interactive) {
        target->setZValue(blueprint.staticZIndex());
        target->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, true);
    }

    //update transform origin
    auto shapeCenter = blueprint.shapeCenter();
    if(shapeCenter.isNull()) {
        auto center = target->boundingRect().center();
        target->setTransformOriginPoint(center);
        
    } else {
        target->setTransformOriginPoint(shapeCenter);   
    }

    //define transparency as it is a dummy
    if(isTargetTemporary) {
        target->setOpacity(.5);
        target->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
    }
            
}

void AtomConverter::_updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom &blueprint) {
    
    //refresh all legal if temporary
    auto paramsToUpdate = blueprint.legalParameters().toList();
    std::sort(paramsToUpdate.begin(), paramsToUpdate.end()); 
    
    //update GI
    for(const auto param : paramsToUpdate) {
        auto val = blueprint.metadata(param);
        _updateGraphicsItemFromMetadata(item, param, val);
    }

}

void AtomConverter::_updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom::Parameter &param, const QVariant &val) {
    
    if(!item) return;
    
    auto requiresTransform = _setParamToGraphicsItemFromAtom(param, item, val);
    if(requiresTransform) _bulkTransformApply(item);

};

void AtomConverter::_bulkTransformApply(QGraphicsItem* itemBrushToUpdate) {
    
    //cast to dest GI type
    auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemBrushToUpdate);
    if(!cItem) return;

    //extract transform instructions
    auto transforms = brushTransform(itemBrushToUpdate);
    
    QTransform toApply;

    //apply transforms to instruction object
    for(auto i = transforms.begin(); i != transforms.end(); i++) {
        
        auto param = static_cast<RPZAtom::Parameter>(i.key().toInt());
        
        switch(param) {
            case RPZAtom::Parameter::AssetScale: {
                auto scaleRatio = i.value().toDouble();
                toApply.scale(scaleRatio, scaleRatio);
            }
            break;

            case RPZAtom::Parameter::AssetRotation: {
                auto degrees = i.value().toInt();
                toApply.rotate(degrees);
            }
            break;

            default:
                break;
        }

    }

    //extract brush type
    auto type = brushDrawStyle(itemBrushToUpdate);
    
    //apply to pen
    if(type == RPZAtom::BrushType::RoundBrush) {
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

bool AtomConverter::_setParamToGraphicsItemFromAtom(const RPZAtom::Parameter &param, QGraphicsItem* itemToUpdate, const QVariant &val) {
    
    switch(param) {
                        
            //on moving
            case RPZAtom::Parameter::Position: {
                auto destPos = val.toPointF();
                MapViewAnimator::animateMove(itemToUpdate, destPos);  
            }
            break;

            // on locking change
            case RPZAtom::Parameter::Locked: {
                auto locked = val.toBool();
                if(RPZClient::isHostAble()) itemToUpdate->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, !locked);
                itemToUpdate->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, !locked);
            }
            break;
            
            // on changing visibility
            case RPZAtom::Parameter::Hidden: {
                if(!isTemporary(itemToUpdate)) {
                    auto hidden = val.toBool();
                    MapViewAnimator::animateVisibility(itemToUpdate, hidden);
                }
            }
            break;

            //on rotation
            case RPZAtom::Parameter::Rotation: {
                auto destRotation = val.toInt();
                itemToUpdate->setRotation(destRotation);
            }
            break;

            //on scaling
            case RPZAtom::Parameter::Scale: {
                auto destScale = val.toDouble();
                itemToUpdate->setScale(destScale);
            }
            break;

            //on text size change
            case RPZAtom::Parameter::TextSize: {
                if(auto cItem = dynamic_cast<QGraphicsTextItem*>(itemToUpdate)) {
                    auto newSize = val.toInt();
                    auto font = cItem->font();
                    font.setPointSize(newSize);
                    cItem->setFont(font);
                    // auto center = cItem->boundingRect().center();
                    // cItem->setTransformOriginPoint(center);
                }

            }
            break;

            //on pen width change
            case RPZAtom::Parameter::BrushPenWidth:
            case RPZAtom::Parameter::PenWidth: {
                if(auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemToUpdate)) {
                    auto newWidth = val.toInt();
                    auto pen = cItem->pen();
                    pen.setWidth(newWidth);
                    cItem->setPen(pen);
                }
            }
            break;

            case RPZAtom::Parameter::NPCAttitude: {
                if(auto cItem = dynamic_cast<MapViewToken*>(itemToUpdate)) {
                    auto attitude = (RPZAtom::NPCType)val.toInt();
                    auto color =  RPZAtom::NPCAssociatedColor(attitude);
                    cItem->updateColor(color);
                }
            }
            break;

            case RPZAtom::Parameter::BrushStyle: {
                if(auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemToUpdate)) {
                    
                    auto type = (RPZAtom::BrushType)val.toInt();
                    setBrushDrawStyle(itemToUpdate, type);
                    
                    //use pen as brush
                    if(type == RPZAtom::BrushType::RoundBrush) {
                        
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
                    if(isTemporary(itemToUpdate)) {
                        
                        QPainterPath path;

                        if(type == RPZAtom::BrushType::Stamp) {
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
            case RPZAtom::Parameter::Text: {
                if(auto cItem = dynamic_cast<QGraphicsTextItem*>(itemToUpdate)) {
                    auto newText = val.toString();
                    cItem->setPlainText(newText);
                }
            }
            break;

            //on layer change
            case RPZAtom::Parameter::Layer: {
                
                auto newLayer = val.toInt();

                //always force temporary item on top of his actual set layer index
                if(isTemporary(itemToUpdate)) newLayer++;

                itemToUpdate->setZValue(newLayer);

            }
            break;
            
            //on asset rotation / scale, store metadata for all-in transform update in main method
            case RPZAtom::Parameter::AssetRotation: {
            case RPZAtom::Parameter::AssetScale: {
                auto transforms = brushTransform(itemToUpdate);
                transforms.insert(QString::number((int)param), val);
                setBrushTransform(itemToUpdate, transforms);
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

void AtomConverter::_setParamToAtomFromGraphicsItem(const RPZAtom::Parameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint) {
    switch(param) {
        
        case RPZAtom::Parameter::Scale: {
            atomToUpdate.setMetadata(param, blueprint->scale());
        }
        break;

        case RPZAtom::Parameter::Rotation: {
            atomToUpdate.setMetadata(param, blueprint->rotation());
        }
        break;

        case RPZAtom::Parameter::TextSize: {
            if(auto casted = dynamic_cast<QGraphicsTextItem*>(blueprint)) {
                atomToUpdate.setMetadata(param, casted->font().pointSize());
            }
        }
        break;

        case RPZAtom::Parameter::Layer: {

            auto layer = blueprint->zValue();

            //if is a temporary, reset layer to expected value
            if(isTemporary(blueprint)) layer--;

            atomToUpdate.setMetadata(param, layer);     

        }
        break;

        case RPZAtom::Parameter::BrushStyle: {
            auto brushStyle = brushDrawStyle(blueprint);
            atomToUpdate.setMetadata(param, (int)brushStyle);
        }
        break;

        case RPZAtom::Parameter::BrushPenWidth:
        case RPZAtom::Parameter::PenWidth: {
            if(auto pathItem = dynamic_cast<MapViewGraphicsPathItem*>(blueprint)) {
                atomToUpdate.setMetadata(param, pathItem->pen().width()); 
            }
        }
        break;

        //add shapeCenter too
        case RPZAtom::Parameter::Shape: {

            if(auto pathItem = dynamic_cast<MapViewGraphicsPathItem*>(blueprint)) {
                auto path = pathItem->path();
                atomToUpdate.setShape(path); 
            } 
            
            else {
                auto rect = blueprint->boundingRect();
                atomToUpdate.setShape(rect);
            }

        }
        break;

        case RPZAtom::Parameter::Position: {
            atomToUpdate.setMetadata(param, blueprint->pos());     
        }
        break;
        
        case RPZAtom::Parameter::AssetScale:
        case RPZAtom::Parameter::AssetRotation: {
            auto transforms = brushTransform(blueprint);
            if(transforms.isEmpty()) return;

            auto transform = transforms.value(QString::number((int)param));
            if(transform.isNull()) return;
            
            atomToUpdate.setMetadata(param, transform); 
        }
        break;

        default:
            break;
    }
}   
