#pragma once

#include "AtomsHandler.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/ui/layout/base/LayerTreeItem.hpp"

#include "src/ui/layout/base/MapLayoutItemDelegate.hpp"

#include "src/shared/commands/AtomsContextualMenuHandler.h"

#include <QTreeWidget>

class TreeMapHint : public AtomsHandler, public AtomsContextualMenuHandler  {
    public:
        TreeMapHint(QTreeWidget* boundTree);

    private:
        QTreeWidget* _boundTree = nullptr;

        bool _preventInnerGIEventsHandling = false;
        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);
        void _updateLayerState(QTreeWidgetItem* layerItem);

        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);
        void _onRenamedAsset(const QString &assetId, const QString &newName);
        
        QHash<snowflake_uid, QTreeWidgetItem*> _treeItemsByAtomId;
        QHash<QString, QSet<snowflake_uid>> _atomIdsBoundByAssetId;
        
        QVector<snowflake_uid> _selectedAtomIds() override;
        snowflake_uid _extractAtomIdFromItem(QTreeWidgetItem* item) const;

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

        void _changeLayer(QVector<snowflake_uid> &elementIds, int newLayer);

        //augmenting AtomsStorage
        virtual void _handlePayload(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const AlterationPayload::Alteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) override;
};