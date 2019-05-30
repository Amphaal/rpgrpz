#pragma once

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
#include <QUuid>
#include <QGraphicsItem>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QHeaderView>

#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/models/RPZAtom.hpp"
#include "src/shared/models/RPZAtom.hpp"

#include "src/ui/map/base/RPZTree.hpp"
#include "LayerTreeItem.hpp"

class MapLayoutTree : public RPZTree {

    Q_OBJECT

    public:
        MapLayoutTree(QWidget* parent = nullptr);
        void alterTreeElements(const AlterationPayload::Alteration &state, QVector<RPZAtom> &elements);
    
    signals:
        void elementsAlterationAsked(const AlterationPayload::Alteration &state, const QVector<QUuid> &elementIds, QVariant &arg = QVariant());

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        void _renderCustomContextMenu(const QPoint &pos);
        void _generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay);
            void _moveSelectionToLayer(int targetLayer);

        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);
        QTreeWidgetItem* _getLayerItem(RPZAtom &atom);
        void _updateLayerState(QTreeWidgetItem* layerItem);

        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);
        void _onRenamedAsset(const QString &assetId, const QString &newName);

        QHash<QUuid, QTreeWidgetItem*> _treeItemsByAtomId;
        QHash<QString, QSet<QTreeWidgetItem*>> _treeItemsByAssetId;

        QVector<QUuid> _extractIdsFromSelection() const;
        AlterationPayload::Alteration _expectedPingback = AlterationPayload::Alteration::Unknown;

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

        void _changeLayer(QVector<QUuid> &elementIds, int newLayer);
        void _changeLayer(RPZAtom &atom);
};