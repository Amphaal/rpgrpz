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

#include <QSharedPointer>

#include "bulk/BulkMetadataChangedPayload.hpp"
#include "wielder/AddedPayload.hpp"
#include "wielder/ResetPayload.hpp"
#include "withTarget/FocusedPayload.hpp"
#include "withTarget/RemovedPayload.hpp"
#include "withTarget/SelectedPayload.hpp"
#include "withTarget/MetadataChangedPayload.hpp"
#include "local/AssetChangedPayload.hpp"
#include "local/AtomTemplateChangedPayload.hpp"
#include "local/AtomTemplateSelectedPayload.hpp"
#include "local/ToySelectedPayload.hpp"
#include "fog/FogChangedPayload.hpp"
#include "fog/FogModeChangedPayload.hpp"

class Payloads {
    public:
        static QSharedPointer<AlterationPayload> autoCast(const QVariantHash &hash) {
            
            AlterationPayload lowCast(hash);
            
            switch(lowCast.type()) {
                
                case Payload::Alteration::FogChanged: {
                    return QSharedPointer<FogChangedPayload>(new FogChangedPayload(hash));
                }

                case Payload::Alteration::FogModeChanged: {
                    return QSharedPointer<FogModeChangedPayload>(new FogModeChangedPayload(hash));
                }

                case Payload::Alteration::BulkMetadataChanged:
                    return QSharedPointer<BulkMetadataChangedPayload>(new BulkMetadataChangedPayload(hash));
                break;

                case Payload::Alteration::AtomTemplateChanged:
                    return QSharedPointer<AtomTemplateChangedPayload>(new AtomTemplateChangedPayload(hash));
                break;

                case Payload::Alteration::AtomTemplateSelected:
                    return QSharedPointer<AtomTemplateSelectedPayload>(new AtomTemplateSelectedPayload(hash));
                break;

                case Payload::Alteration::AssetChanged:
                    return QSharedPointer<AssetChangedPayload>(new AssetChangedPayload(hash));
                break;

                case Payload::Alteration::ToySelected:
                    return QSharedPointer<ToySelectedPayload>(new ToySelectedPayload(hash));
                break;

                case Payload::Alteration::Added:
                    return QSharedPointer<AddedPayload>(new AddedPayload(hash));
                break;

                case Payload::Alteration::Reset:
                    return QSharedPointer<ResetPayload>(new ResetPayload(hash));
                break;

                case Payload::Alteration::Focused:
                    return QSharedPointer<FocusedPayload>(new FocusedPayload(hash));
                break;

                case Payload::Alteration::Removed:
                    return QSharedPointer<RemovedPayload>(new RemovedPayload(hash));
                break;

                case Payload::Alteration::Selected:
                    return QSharedPointer<SelectedPayload>(new SelectedPayload(hash));
                break;

                case Payload::Alteration::MetadataChanged:
                    return QSharedPointer<MetadataChangedPayload>(new MetadataChangedPayload(hash));
                break;

                default:
                    return QSharedPointer<AlterationPayload>(new AlterationPayload(hash));
                break;
                
            }

        }
};