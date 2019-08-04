#pragma once

#include "AtomsHandler.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/ui/layout/base/LayerTreeItem.h"
#include "src/ui/layout/base/MapLayoutItemDelegate.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"

#include <QTreeWidget>

class TreeMapHint : public AtomsHandler, public AtomsContextualMenuHandler {
    
    Q_OBJECT
    
    public:
        TreeMapHint(AtomsStorage* mapMaster);

        void _updateLayerState(QTreeWidgetItem* layerItem);

    public slots:
        void propagateFocus(snowflake_uid focusedAtomId);
        void propagateSelection(QVector<snowflake_uid> &selectedIds);

    signals:
        void requestingTreeItemInsertion(QTreeWidgetItem *item, QTreeWidgetItem* parent);

    private:
        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);

        void _bindOwnerToItem(QTreeWidgetItem* item, RPZUser &owner);

        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);
        void _onRenamedAsset(const RPZAssetHash &id, const QString &newName);
        
        QHash<snowflake_uid, QTreeWidgetItem*> _atomTreeItemsById;
        QHash<RPZAssetHash, QSet<snowflake_uid>> _atomIdsBoundByRPZAssetHash;

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

        void _changeLayer(QVector<snowflake_uid> &elementIds, int newLayer);

        //augmenting AtomsStorage
        virtual void _handlePayload(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) override;

        //icons
        QIcon* _layerIcon = nullptr;
        QIcon* _textIcon = nullptr;
        QIcon* _drawingIcon = nullptr;
};