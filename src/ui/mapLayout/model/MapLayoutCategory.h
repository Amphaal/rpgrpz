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