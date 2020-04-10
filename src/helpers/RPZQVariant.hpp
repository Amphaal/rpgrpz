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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QListWidgetItem>
#include <QGraphicsItem>

#include <audiotube/VideoMetadata.h>
#include "src/shared/models/RPZAtom.h"

class RPZQVariant {
    public:
        using AtomTransformationList = QHash<RPZAtom::Parameter, QVariant>;

        enum class Roles {
            AtomId = 1006,
            YTVideoMetadataPtr = 1007,
            GridAlignable = 1100,
            BrushTransform = 2555, 
            IsTemporary = 2666, 
            IsManuallyHidden = 2680, 
            BrushDrawStyle = 2767,
            CachedOpacity = 6000,
            ContextuallyOwned = 7000,
            AllowedToDefineMoveAbility = 8000,
            CharacterId = 8010,
            CoveredByFog = 9000,
            FogSensitive = 9010,
            GraphicsItemToReplace = 10100,
            MoveAnimationDestinationScenePoint = 11000,
            OverlappingGridBound = 12000
        };

        static QVariant moveAnimationDestinationScenePoint(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::MoveAnimationDestinationScenePoint);
        }
        static void setMoveAnimationDestinationScenePoint(QGraphicsItem* item, const QPointF &sceneDestPoint) {
            item->setData((int)RPZQVariant::Roles::MoveAnimationDestinationScenePoint, QVariant::fromValue<QPointF>(sceneDestPoint));
        }
        static void deleteMoveAnimationDestinationScenePoint(QGraphicsItem* item) {
            item->setData((int)RPZQVariant::Roles::MoveAnimationDestinationScenePoint, QVariant());
        }

        static QGraphicsItem* graphicsItemToReplace(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::GraphicsItemToReplace).value<QGraphicsItem*>();
        }
        static void setGraphicsItemToReplace(QGraphicsItem* item, QGraphicsItem* toReplace) {
            item->setData((int)RPZQVariant::Roles::GraphicsItemToReplace, QVariant::fromValue<QGraphicsItem*>(toReplace));
        }

        static void setContextuallyOwned(QGraphicsItem* item, bool owned) {
            item->setData((int)RPZQVariant::Roles::ContextuallyOwned, owned);
        }
        static bool contextuallyOwned(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::ContextuallyOwned).toBool();
        }

        static bool allowedToDefineMoveAbility(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AllowedToDefineMoveAbility).toBool();
        }
        static void setAllowedToDefineMoveAbility(QGraphicsItem* item, bool allowed) {
            item->setData((int)RPZQVariant::Roles::AllowedToDefineMoveAbility, allowed);
        }

        static VideoMetadata* ytVideoMetadata(const QListWidgetItem* item) {
            return item->data((int)RPZQVariant::Roles::YTVideoMetadataPtr).value<VideoMetadata*>();
        }
        static void setYTVideoMetadata(QListWidgetItem* item, VideoMetadata* metadata) {
            item->setData((int)RPZQVariant::Roles::YTVideoMetadataPtr, QVariant::fromValue<VideoMetadata*>(metadata));
        }

        static RPZAtom::Id atomId(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::AtomId).toULongLong();
        }
        static void setAtomId(QGraphicsItem* item, const RPZAtom::Id &id) {
            item->setData((int)RPZQVariant::Roles::AtomId, QVariant::fromValue<RPZAtom::Id>(id));
        }

        static RPZCharacter::Id boundCharacterId(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::CharacterId).toULongLong();
        }
        static void setBoundCharacterId(QGraphicsItem* item, const RPZCharacter::Id &characterId) {
            item->setData((int)RPZQVariant::Roles::CharacterId, QVariant::fromValue<RPZCharacter::Id>(characterId));
        }

        static auto brushTransform(QGraphicsItem *item) {
            
            auto rawHash = item->data((int)RPZQVariant::Roles::BrushTransform).toHash();
            
            AtomTransformationList out;
            
            for(auto i = rawHash.begin(); i != rawHash.end(); i++) {
                auto param = static_cast<RPZAtom::Parameter>(i.key().toInt());
                out.insert(param, i.value());
            }

            return out;

        }
        static void setBrushTransform(QGraphicsItem *item, const AtomTransformationList &transforms) {

            QVariantHash out;
            for(auto i = transforms.begin(); i != transforms.end(); i++) {
                auto toStr = QString::number((int)i.key());
                out.insert(toStr, i.value());
            }

            item->setData((int)RPZQVariant::Roles::BrushTransform, out);
            
        }

        static bool isTemporary(QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::IsTemporary).toBool();
        }
        static void setIsTemporary(QGraphicsItem* item, bool isTemporary) {
            item->setData((int)RPZQVariant::Roles::IsTemporary, isTemporary);
        }

        static bool isManuallyHidden(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::IsManuallyHidden).toBool();
        }
        static void setIsManuallyHidden(QGraphicsItem* item, bool isHidden) {
            item->setData((int)RPZQVariant::Roles::IsManuallyHidden, isHidden);
        }

        static void setIsCoveredByFog(QGraphicsItem* item, bool isCovered) {
            item->setData((int)RPZQVariant::Roles::CoveredByFog, isCovered);
        }
        static bool isCoveredByFog(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::CoveredByFog).toBool();
        }

        static void setFogSensitive(QGraphicsItem* item, bool isFogSensitive) {
            item->setData((int)RPZQVariant::Roles::FogSensitive, isFogSensitive);
        }
        static bool fogSensitive(const QGraphicsItem* item) {
            return item->data((int)RPZQVariant::Roles::FogSensitive).toBool();
        }

        static double cachedOpacity(QGraphicsItem* item) {
            bool ok = false;
            auto data = item->data((int)RPZQVariant::Roles::CachedOpacity).toDouble(&ok);
            return ok ? data : 1;
        }
        static void setCachedOpacity(QGraphicsItem* item, double cached) {
            item->setData((int)RPZQVariant::Roles::CachedOpacity, cached);
        }

        static RPZAtom::BrushType brushDrawStyle(QGraphicsItem* item) {
            return (RPZAtom::BrushType)item->data((int)RPZQVariant::Roles::BrushDrawStyle).toInt();
        }
        static void setBrushDrawStyle(QGraphicsItem* item, const RPZAtom::BrushType &style) {
            item->setData((int)RPZQVariant::Roles::BrushDrawStyle, (int)style);
        }


};
inline uint qHash(const RPZQVariant::Roles &key, uint seed = 0) {return uint(key) ^ seed;}
