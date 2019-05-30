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

#include "src/shared/network/RPZAtom.hpp"
#include "src/ui/map/base/RPZTree.hpp"
#include "LayerTreeItem.hpp"

class MapLayoutTree : public RPZTree {

    Q_OBJECT

    public:
        MapLayoutTree(QWidget* parent = nullptr);
        void alterTreeElements(const RPZAtom::Alteration &state, QVector<RPZAtom> &elements);
    
    signals:
        void elementsAlterationAsked(const RPZAtom::Alteration &state, const QVector<QUuid> &elementIds, QVariant &arg = QVariant());

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

        QHash<QUuid, QTreeWidgetItem*> _treeItemsById;
        QVector<QUuid> _extractIdsFromSelection() const;
        RPZAtom::Alteration _expectedPingback = RPZAtom::Alteration::Unknown;

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

        void _changeLayer(QVector<QUuid> &elementIds, int newLayer);
        void _changeLayer(RPZAtom &atom);
};