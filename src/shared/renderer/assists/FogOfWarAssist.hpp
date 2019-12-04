#pragma once

#include <QGraphicsView>

#include "src/shared/renderer/graphics/MapViewGraphics.h"

class FogOfWarAssist {
    public:
        FogOfWarAssist(QGraphicsView* view) : _view(view) {}

        MapViewFog* generateFOWItem() {
            return new MapViewFog(this->_view);
        }
    
    private:
        MapViewFog* _fog = nullptr;
        QGraphicsView* _view = nullptr;
};