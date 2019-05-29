#pragma once

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
#include <QUuid>
#include <QGraphicsItem>
#include <QIcon>

#include "MapView.h"
#include "src/shared/network/RPZAsset.hpp"
#include "base/RPZTree.hpp"

class MapLayoutManager : public RPZTree {

    Q_OBJECT

    public:
        MapLayoutManager(QWidget* parent = nullptr);
        void alterTreeElements(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements);
    
    signals:
        void elementsAlterationAsked(const RPZAsset::Alteration &state, const QVector<QUuid> &elementIds, QVariant &arg = QVariant());

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        void _renderCustomContextMenu(const QPoint &pos);
        void _generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay);
            void _moveSelectionToLayer(int targetLayer);

        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);
        QTreeWidgetItem* _getLayerItem(RPZAsset &asset);
        void _updateLayerState(QTreeWidgetItem* layerItem);

        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);

        QHash<QUuid, QTreeWidgetItem*> _treeItemsById;
        QVector<QUuid> _extractIdsFromSelection() const;
        RPZAsset::Alteration _expectedPingback = RPZAsset::Alteration::Unknown;

        QTreeWidgetItem* _createTreeItem(RPZAsset &asset);

        void _changeLayer(QVector<QUuid> &elementIds, int newLayer);
        void _changeLayer(RPZAsset &asset);
};