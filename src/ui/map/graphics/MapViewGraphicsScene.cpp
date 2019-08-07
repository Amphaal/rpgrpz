#include "MapViewGraphicsScene.h"

MapViewGraphicsScene::MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

void MapViewGraphicsScene::_onGraphicsItemCustomChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) {
    emit sceneItemChanged(item, (int)flag);
};