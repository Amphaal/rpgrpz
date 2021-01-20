// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#pragma once

#include "MapLayoutItem.hpp"
#include "src/shared/models/RPZAtom.h"

class MapLayoutAtom;
class MapLayoutCategory : public MapLayoutItem {
 public:
    MapLayoutCategory(const RPZAtom::Category &category, int sorter);
    int sorter() const;
    RPZAtom::Category category() const;

    static MapLayoutCategory* fromIndex(const QModelIndex &index);

    bool operator<(const MapLayoutCategory &other) const;

    void addAsChild(MapLayoutAtom* child);

    void removeAsChild(MapLayoutAtom* child);

    void updateSorter(RPZAtom::Layer newLayer);

    int atomsCount() const;

    MapLayoutAtom* atom(int row) const;

    int rowOfAtom(MapLayoutAtom* atomToFind) const;

    const QString name() const override;

    const QPixmap icon() const override;

 private:
    QVector<MapLayoutAtom*> _atoms;
    int _sorter = 0;
    RPZAtom::Category _category = (RPZAtom::Category)0;
    QString _name;
    QPixmap _pixmap;

    void _updateLayerName(RPZAtom::Layer newLayer);
};
