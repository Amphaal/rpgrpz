#pragma once

#include "src/shared/renderer/graphics/_generic/MapViewGraphicsPathItem.hpp"

class MapViewDrawing : public MapViewGraphicsPathItem {
    public:
        MapViewDrawing(const QPainterPath &path, const QPen &pen, const QBrush &brush = QBrush()) : MapViewGraphicsPathItem(path, pen, brush) {}
};