#pragma once

#include <QGraphicsView>

class MV_HUDLayout {
    public:
        MV_HUDLayout(QGraphicsView* view) : _view(view) {

        }
    
    private:
    QGraphicsView* _view = nullptr;

};