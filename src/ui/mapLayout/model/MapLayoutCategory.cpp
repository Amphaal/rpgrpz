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

#include "MapLayoutCategory.h"
#include "MapLayoutAtom.h"

MapLayoutCategory::MapLayoutCategory(const RPZAtom::Category &category, int sorter) {
    this->_category = category;
    this->_sorter = sorter;

    switch(category) {
        case RPZAtom::Category::Layout: {
            this->_updateLayerName(sorter);
            this->_pixmap = QPixmap(QStringLiteral(u":/icons/app/manager/layer.png"));
        }
        break;

        case RPZAtom::Category::Interactive: {
            auto type = (RPZAtom::Type)sorter;
            this->_name = QObject::tr(qUtf8Printable(RPZAtom::atomCategoryTypeDescr.value(type)));
            this->_pixmap = QPixmap(RPZAtom::iconPathByAtomType.value(type));
        }
        break;

        default:
        break;

    }
}

int MapLayoutCategory::rowOfAtom(MapLayoutAtom* atom) const {
    return this->_atoms.indexOf(atom);
}

void MapLayoutCategory::updateSorter(RPZAtom::Layer newLayer) {
    this->_sorter = newLayer;
    this->_updateLayerName(newLayer);
}

void MapLayoutCategory::_updateLayerName(RPZAtom::Layer newLayer) {
    this->_name = QObject::tr("Layer %1").arg(newLayer);
}

int MapLayoutCategory::sorter() const {return this->_sorter;}
RPZAtom::Category MapLayoutCategory::category() const {return this->_category;}

MapLayoutCategory* MapLayoutCategory::fromIndex(const QModelIndex &index) {
    auto ip = index.internalPointer();
    return static_cast<MapLayoutCategory*>(ip);
};

bool MapLayoutCategory::operator<(const MapLayoutCategory &other) const {
    return this->sorter() < other.sorter();
}

void MapLayoutCategory::addAsChild(MapLayoutAtom* child) {
    this->_atoms.append(child);
};

void MapLayoutCategory::removeAsChild(MapLayoutAtom* child) {
    this->_atoms.removeOne(child);
}

int MapLayoutCategory::atomsCount() const {
    return this->_atoms.count();
}

MapLayoutAtom* MapLayoutCategory::atom(int row) const {
    return this->_atoms.at(row);
}

const QString MapLayoutCategory::name() const {
    return this->_name;
}

const QPixmap MapLayoutCategory::icon() const {
    return this->_pixmap;
};
