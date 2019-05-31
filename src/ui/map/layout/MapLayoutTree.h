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

#include "src/shared/models/entities/RPZAtom.hpp"
#include "src/shared/models/Payloads.h"

#include "src/ui/map/base/RPZTree.hpp"
#include "LayerTreeItem.hpp"

class MapLayoutTree : public RPZTree {

    Q_OBJECT

    public:
        MapLayoutTree(QWidget* parent = nullptr);
    
    public slots:
        void alterTreeElements(QVariantHash &payload);
    
    signals:
        void elementsAlterationAsked(QVariantHash &payload);

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        AlterationPayload::Source _source = AlterationPayload::Source::Local_MapLayout;

        void _emitAlteration(AlterationPayload &payload);

        void _renderCustomContextMenu(const QPoint &pos);
        void _generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay);
            void _moveSelectionToLayer(int targetLayer);

        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);
        void _updateLayerState(QTreeWidgetItem* layerItem);

        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);
        void _onRenamedAsset(const QString &assetId, const QString &newName);

        QHash<QUuid, QTreeWidgetItem*> _treeItemsByAtomId;
        QHash<QString, QSet<QUuid>> _atomIdsBoundByAssetId;
        
        QVector<QUuid> _selectedAtomIds() const;
        QUuid _extractAtomIdFromItem(QTreeWidgetItem* item) const;

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

        void _changeLayer(QVector<QUuid> &elementIds, int newLayer);
};