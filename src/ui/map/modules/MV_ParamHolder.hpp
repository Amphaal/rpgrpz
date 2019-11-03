#pragma once

#include "src/shared/database/MapDatabase.h"
#include <QGraphicsView>

class MV_ParamHolder {
    public:
        MV_ParamHolder() {

        }
    
    private:
        QGraphicsView* _view;
        MapDatabase* _db;
};