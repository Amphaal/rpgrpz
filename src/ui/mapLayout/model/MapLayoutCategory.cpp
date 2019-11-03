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

const int MapLayoutCategory::rowOfAtom(MapLayoutAtom* atom) const {
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

const int MapLayoutCategory::atomsCount() const {
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
