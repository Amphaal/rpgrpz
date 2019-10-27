#pragma once

#include "MapLayoutItem.hpp"
#include "src/shared/models/RPZAtom.h"

class MapLayoutAtom;
class MapLayoutCategory : public MapLayoutItem {
    public:
        MapLayoutCategory(RPZAtomLayer layer) : MapLayoutCategory(RPZAtom::Category::Layout, layer) {
            this->_name = QObject::tr("Layer %1");
            this->_pixmap = QPixmap(QStringLiteral(u":/icons/app/manager/layer.png"));
        }
        MapLayoutCategory(RPZAtomType type) : MapLayoutCategory(RPZAtom::Category::Interactive, (int)type) {
            this->_name = RPZAtom::toString(type);
            this->_pixmap = QPixmap(RPZAtom::iconPathByAtomType.value(type));
        }

        int sorter() const {return this->_sorter;}
        RPZAtom::Category category() const {return this->_category;}

        static MapLayoutCategory* fromIndex(const QModelIndex &index) {
            auto ip = index.internalPointer();
            return static_cast<MapLayoutCategory*>(ip);
        };

        bool operator<(const MapLayoutCategory &other) const {
            return this->sorter() < other.sorter();
        }

        void addAsChild(MapLayoutAtom* child) {
            child->setParent(this);
            this->_atoms.insert(child);
        };

        void removeAsChild(MapLayoutAtom* child) {
            this->_atoms.remove(child);
        }

        const QSet<MapLayoutAtom*>& atoms() const {
            return this->_atoms;
        }

        MapLayoutAtom* atom(int row) const {
            auto internal = -1;
            
            for(auto atom : this->_atoms) {
                internal++;
                if(internal == row) return atom;
            }

            return nullptr;
        }

        const QString name() const override {
            return this->_name;
        }

        const QPixmap icon() const override {
            return this->_pixmap;
        };

    private:
        QSet<MapLayoutAtom*> _atoms;
        int _sorter = 0;
        RPZAtom::Category _category = (RPZAtom::Category)0;
        QString _name;
        QPixmap _pixmap;

        MapLayoutCategory(const RPZAtom::Category &category, int sorter) {
            this->_category = _category;
            this->_sorter = sorter;
        }
    
};