#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "layout/MapLayoutTree.h"
#include "layout/LayerSelector.h"
#include "layout/AtomEditor.h"

class MapLayoutManager : public QWidget {
    public:
        MapLayoutManager(QWidget *parent = nullptr);

        MapLayoutTree* tree();
        LayerSelector* layerSelector();
        AtomEditor* editor();
    
    private:
        MapLayoutTree* _tree = nullptr;
        LayerSelector* _layerSelector = nullptr;
        AtomEditor* _editor = nullptr;
};