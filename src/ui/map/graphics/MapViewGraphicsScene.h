#pragma once 

#include <QGraphicsScene>

#include "MapViewItemsNotifier.h"

class  MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        void onGraphicsItemCustomChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override;

    public:
        MapViewGraphicsScene(int defaultSize);
};