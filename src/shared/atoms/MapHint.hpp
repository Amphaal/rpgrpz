#pragma once

#include "ViewMapHint.h"

class MapHint : public ViewMapHint {
    public:
        MapHint(QGraphicsView* boundGv) : ViewMapHint(boundGv) { }
};