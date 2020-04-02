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

#pragma once

#include "MapLayoutItem.hpp"

#include "src/shared/models/RPZAtom.h"

class MapLayoutCategory;
class MapLayoutAtom : public MapLayoutItem {
    public:
        MapLayoutAtom(MapLayoutCategory* parent, const RPZAtom &atom);
        ~MapLayoutAtom();

        static MapLayoutAtom* fromIndex(const QModelIndex &index);

        void setParent(MapLayoutCategory* parent);
        MapLayoutCategory* parent() const;

        const QHash<int, QSet<int>> updateFrom(const RPZAtom::Updates &updates);

        bool isHidden() const;
        bool isLocked() const;
        const RPZAsset::Hash assetHash() const;
        RPZAtom::Id atomId() const;
        const QString name() const override;
        const QPixmap icon() const override;

        bool notifyAssetNameChange(const QString newAssetName);

    private:
        MapLayoutCategory* _parent = nullptr;
        QString _name;
        RPZAtom::Type _type = (RPZAtom::Type)0;
        bool _isHidden = false;
        bool _isLocked = false;
        RPZAtom::Id _id = 0;
        RPZAsset::Hash _assetHash;
        QString _iconPath;

        RPZAtom::Parameter _nameChangeParam;

};