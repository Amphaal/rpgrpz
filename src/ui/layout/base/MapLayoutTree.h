#pragma once

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
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

#include "src/ui/others/ClientBindable.hpp"

#include "MapLayoutItemDelegate.hpp"

class MapLayoutTree : public RPZTree, ClientBindable {

    Q_OBJECT

    public:
        MapLayoutTree(QWidget* parent = nullptr);
    
    public slots:
        void alterTreeElements(QVariantHash &payload);
    
    signals:
        void elementsAlterationAsked(QVariantHash &payload);

    protected:
        void keyPressEvent(QKeyEvent * event) override;
        void onRPZClientConnecting(RPZClient * cc) override;

    private:
        bool _preventInnerGIEventsHandling = false;
        AlterationPayload::Source _source = AlterationPayload::Source::Local_MapLayout;

        void _emitAlteration(AlterationPayload &payload);

        void _renderCustomContextMenu(const QPoint &pos);
        void _generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay);
            QList<QAction*> _genLayerActions(QList<QTreeWidgetItem*> &selectedItems);
            QList<QAction*> _genVisibilityActions(QList<QTreeWidgetItem*> &selectedItems);
            QList<QAction*> _genAvailabilityActions(QList<QTreeWidgetItem*> &selectedItems);

        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);
        void _updateLayerState(QTreeWidgetItem* layerItem);

        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);
        void _onRenamedAsset(const QString &assetId, const QString &newName);
        
        QHash<snowflake_uid, QTreeWidgetItem*> _treeItemsByAtomId;
        QHash<QString, QSet<snowflake_uid>> _atomIdsBoundByAssetId;
        
        QVector<snowflake_uid> _selectedAtomIds() const;
        snowflake_uid _extractAtomIdFromItem(QTreeWidgetItem* item) const;

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

        void _changeLayer(QVector<snowflake_uid> &elementIds, int newLayer);
};