#pragma once

#include <QAbstractItemModel>

#include "MapLayoutItem.hpp"

typedef QHash<RPZAtomLayer, MapLayoutItem*> MapLayoutHash;
typedef QHash<RPZAtomType, MapLayoutItem*> MapInteractiveHash;

class MapLayoutModel : public QAbstractItemModel {
    public:
        MapLayoutModel() {

        }
    
    private:
        MapLayoutHash _atomsByLayer;
        MapInteractiveHash _atomsByType;
};