#pragma once 

#include <QGraphicsScene>

#include <QFileInfo>

#include "items/MapViewGraphicsItems.h"
#include "MapViewItemsNotifier.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/AssetMetadata.hpp"

#include "src/helpers/AtomConverter.h"

class MapViewGraphicsScene : public QGraphicsScene, MapViewItemsNotified {

    Q_OBJECT

    signals:
        void sceneItemChanged(QGraphicsItem* item, int atomAlteration);

    private:
        void onGraphicsItemCustomChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) override;

    public:
        MapViewGraphicsScene(int defaultSize);

        QGraphicsItem* addToScene(RPZAtom &atom, AssetMetadata &assetMetadata, bool isTemporary = false);
        QGraphicsRectItem* addMissingAssetPH(RPZAtom &atom);

    protected:
        QGraphicsItem* _addGenericImageBasedItem(RPZAtom &atom, AssetMetadata &assetMetadata);
        QGraphicsPathItem* _addBrush(RPZAtom &atom, AssetMetadata &assetMetadata);
        QGraphicsPathItem* _addDrawing(RPZAtom &atom);
        QGraphicsTextItem* _addText(RPZAtom &atom);
};