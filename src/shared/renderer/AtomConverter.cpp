// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "AtomConverter.h"

void AtomConverter::updateGraphicsItemFromMetadata(QGraphicsItem* item, const RPZAtom::Updates &updates) {
    // update GI
    for (auto i = updates.begin(); i != updates.end(); i++) {
        _updateGraphicsItemFromAtomParamValue(item, i.key(), i.value());
    }
}

const RPZAtom AtomConverter::cloneAtomTemplateFromGraphics(QGraphicsItem* blueprint, RPZAtom toClone) {
    // update the 2 only parameters who might have changed from the template
    _updateAtomParamFromGraphics(RPZAtom::Parameter::Position, toClone, blueprint);
    _updateAtomParamFromGraphics(RPZAtom::Parameter::Shape, toClone, blueprint);

    // finally, give it ID for storage
    toClone.shuffleId();

    return toClone;
}

void AtomConverter::setupGraphicsItemFromAtom(QGraphicsItem* target, const RPZAtom &blueprint) {
    // set default movable
    auto default_canBeMoved = Authorisations::isHostAble() && !blueprint.isWalkableAtom();
    target->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, default_canBeMoved);
    RPZQVariant::setAllowedToDefineMoveAbility(target, default_canBeMoved);

    // set fog of war sensibility
    auto isFogSensitive = !RPZAtom::fogOfWarInsensitive.contains(blueprint.type());
    RPZQVariant::setFogSensitive(target, isFogSensitive);

    // update
    _updateGraphicsItemFromBlueprint(target, blueprint);

    // if interactive, force ZIndex to max
    if (blueprint.category() == RPZAtom::Category::Interactive) {
        target->setZValue(blueprint.staticZIndex());
        target->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, true);
    }

    // update transform origin
    auto shapeCenter = blueprint.shapeCenter();
    if (shapeCenter.isNull()) {
        auto center = target->boundingRect().center();
        target->setTransformOriginPoint(center);
    } else {
        target->setTransformOriginPoint(shapeCenter);
    }

    // define transparency as it is a dummy
    if (RPZQVariant::isTemporary(target)) {
        target->setOpacity(.5);
        target->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
        target->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
    }
}

void AtomConverter::_updateGraphicsItemFromBlueprint(QGraphicsItem* item, const RPZAtom &blueprint) {
    // refresh all legal if temporary
    auto paramsToUpdate = blueprint.legalParameters();

    // if temporary, remove Hidden and Opacity
    auto isItemTemporary = RPZQVariant::isTemporary(item);
    if (isItemTemporary) {
        paramsToUpdate.remove(RPZAtom::Parameter::Opacity);
        paramsToUpdate.remove(RPZAtom::Parameter::Hidden);
    }

    // sort
    auto asList = paramsToUpdate.values();
    std::sort(asList.begin(), asList.end());

    // update GI
    for (const auto param : asList) {
        auto val = blueprint.metadata(param);
        _updateGraphicsItemFromAtomParamValue(item, param, val);
    }
}

void AtomConverter::_updateGraphicsItemFromAtomParamValue(QGraphicsItem* item, const RPZAtom::Parameter &param, const QVariant &val) {
    if (!item) return;

    auto requiresTransform = _setParamToGraphicsItemFromAtom(param, item, val);
    if (requiresTransform) _bulkTransformApply(item);
}

void AtomConverter::_bulkTransformApply(QGraphicsItem* itemBrushToUpdate) {
    // cast to dest GI type
    auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemBrushToUpdate);
    if (!cItem) return;

    // extract transform instructions
    auto transforms = RPZQVariant::brushTransform(itemBrushToUpdate);

    QTransform toApply;

    // apply transforms to instruction object
    for (auto i = transforms.begin(); i != transforms.end(); i++) {
        switch (i.key()) {
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

    // extract brush type
    auto type = RPZQVariant::brushDrawStyle(itemBrushToUpdate);

    // apply to pen
    if (type == RPZAtom::BrushType::RoundBrush) {
        auto pen = cItem->pen();
        auto brush = pen.brush();
        brush.setTransform(toApply);
        pen.setBrush(brush);
        cItem->setPen(pen);
    } else {  // apply to GI brush
        auto brush = cItem->brush();
        brush.setTransform(toApply);
        cItem->setBrush(brush);
    }
}

bool AtomConverter::_setParamToGraphicsItemFromAtom(const RPZAtom::Parameter &param, QGraphicsItem* itemToUpdate, const QVariant &val) {
    switch (param) {
            case RPZAtom::Parameter::TokenSize: {
                if (auto cItem = dynamic_cast<MapViewToken*>(itemToUpdate)) {
                    auto type = (RPZAtom::TokenSize)val.toInt();
                    cItem->updateTokenSize(type);
                }
            }
            break;

            case RPZAtom::Parameter::PenColor: {
                if (auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemToUpdate)) {
                    auto newColor = val.value<QColor>();
                    auto pen = cItem->pen();
                    pen.setColor(newColor);
                    cItem->setPen(pen);
                }
            }
            break;

            // on moving
            case RPZAtom::Parameter::Position: {
                auto destPos = val.toPointF();
                MapViewAnimator::animateMove(itemToUpdate, destPos);
            }
            break;

            // on locking change
            case RPZAtom::Parameter::Locked: {
                if (!RPZQVariant::allowedToDefineMoveAbility(itemToUpdate)) break;
                auto locked = val.toBool();
                itemToUpdate->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, !locked);
                itemToUpdate->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, !locked);
            }
            break;

            // on covering changed
            case RPZAtom::Parameter::CoveredByFog: {
                auto isCovered = val.toBool();
                RPZQVariant::setIsCoveredByFog(itemToUpdate, isCovered);

                // animate
                MapViewAnimator::animateVisibility(itemToUpdate);
            }
            break;

            // on changing visibility
            case RPZAtom::Parameter::Hidden: {
                // define visibility
                auto hidden = val.toBool();
                RPZQVariant::setIsManuallyHidden(itemToUpdate, hidden);

                // animate
                MapViewAnimator::animateVisibility(itemToUpdate);
            }
            break;

            // on changing opacity
            case RPZAtom::Parameter::Opacity: {
                // define cached opacity on item
                auto opacity = (double)val.toInt() / 100;
                RPZQVariant::setCachedOpacity(itemToUpdate, opacity);

                // prevent changing opacity if is hidden
                if (RPZQVariant::isManuallyHidden(itemToUpdate)) break;

                // animate
                MapViewAnimator::animateVisibility(itemToUpdate);
            }
            break;

            // on rotation
            case RPZAtom::Parameter::Rotation: {
                auto destRotation = val.toInt();
                itemToUpdate->setRotation(destRotation);
            }
            break;

            // on scaling
            case RPZAtom::Parameter::Scale: {
                auto destScale = val.toDouble();
                itemToUpdate->setScale(destScale);
            }
            break;

            // on text size change
            case RPZAtom::Parameter::TextSize: {
                if (auto cItem = dynamic_cast<QGraphicsTextItem*>(itemToUpdate)) {
                    auto newSize = val.toInt();
                    auto font = cItem->font();
                    font.setPointSize(newSize);
                    cItem->setFont(font);
                    // auto center = cItem->boundingRect().center();
                    // cItem->setTransformOriginPoint(center);
                }
            }
            break;

            // on pen width change
            case RPZAtom::Parameter::BrushPenWidth:
            case RPZAtom::Parameter::PenWidth: {
                if (auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemToUpdate)) {
                    auto newWidth = val.toInt();
                    auto pen = cItem->pen();
                    pen.setWidth(newWidth);
                    cItem->setPen(pen);
                }
            }
            break;

            case RPZAtom::Parameter::DefaultPlayerColor: {
                if (auto cItem = dynamic_cast<MapViewToken*>(itemToUpdate)) {
                    auto color = val.value<QColor>();
                    cItem->updateCharacterBoundColor(color);
                }
            }
            break;

            case RPZAtom::Parameter::NPCAttitude: {
                if (auto cItem = dynamic_cast<MapViewToken*>(itemToUpdate)) {
                    auto attitude = (RPZAtom::NPCType)val.toInt();
                    cItem->updateColorFromAttitude(attitude);
                }
            }
            break;

            case RPZAtom::Parameter::BrushStyle: {
                if (auto cItem = dynamic_cast<MapViewGraphicsPathItem*>(itemToUpdate)) {
                    auto type = (RPZAtom::BrushType)val.toInt();
                    RPZQVariant::setBrushDrawStyle(itemToUpdate, type);

                    // use pen as brush
                    if (type == RPZAtom::BrushType::RoundBrush) {
                        // reset brush
                        cItem->setBrush(QBrush());

                        // set pen
                        auto pen = cItem->pen();
                        pen.setBrush(cItem->sourceBrush());
                        cItem->setPen(pen);
                    } else {  // use item brush
                        // reset pen
                        auto pen = cItem->pen();
                        pen.setBrush(QBrush());
                        cItem->setPen(pen);

                        // set brush
                        cItem->setBrush(cItem->sourceBrush());
                    }

                    // define default shape for temporary
                    if (RPZQVariant::isTemporary(itemToUpdate)) {
                        QPainterPath path;

                        if (type == RPZAtom::BrushType::Stamp) {
                            QRectF rect(QPointF(0, 0), cItem->sourceBrushSize());
                            path.addRect(rect);
                        } else {
                            path.lineTo(.01, .01);
                        }

                        cItem->setPath(path);
                    }
                }
            }
            break;

            // on text change
            case RPZAtom::Parameter::Text: {
                if (auto cItem = dynamic_cast<QGraphicsTextItem*>(itemToUpdate)) {
                    auto newText = val.toString();
                    cItem->setPlainText(newText);
                }
            }
            break;

            // on character id change
            case RPZAtom::Parameter::CharacterId: {
                RPZQVariant::setBoundCharacterId(itemToUpdate, val.toULongLong());
            }
            break;

            // on layer change
            case RPZAtom::Parameter::Layer: {
                auto newLayer = val.toInt();

                // always force temporary item on top of his actual set layer index
                if (RPZQVariant::isTemporary(itemToUpdate)) newLayer++;

                itemToUpdate->setZValue(newLayer);
            }
            break;

            // on asset rotation / scale, store metadata for all-in transform update in main method
            case RPZAtom::Parameter::AssetRotation: {
            case RPZAtom::Parameter::AssetScale: {
                auto transforms = RPZQVariant::brushTransform(itemToUpdate);
                transforms.insert(param, val);
                RPZQVariant::setBrushTransform(itemToUpdate, transforms);
                return true;
            }
            break;

            default:
                break;
        }
    }

    // no transform to apply
    return false;
}

void AtomConverter::_updateAtomParamFromGraphics(const RPZAtom::Parameter &param, RPZAtom &atomToUpdate, QGraphicsItem* blueprint) {
    switch (param) {
        // add shapeCenter too
        case RPZAtom::Parameter::Shape: {
            if (auto pathItem = dynamic_cast<MapViewGraphicsPathItem*>(blueprint)) {
                auto path = pathItem->path();
                atomToUpdate.setShape(path);
            } else {
                auto rect = blueprint->boundingRect();
                atomToUpdate.setShape(rect);
            }
        }
        break;

        case RPZAtom::Parameter::Position: {
            atomToUpdate.setMetadata(param, blueprint->pos());
        }
        break;

        default: {
            qWarning() << "Unhandled RPZAtom::Parameter to update from !";
        }
        break;
    }
}

