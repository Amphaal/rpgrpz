#pragma once

#include "src\shared\async-ui\AlterationAcknoledger.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/ui/layout/base/LayerTreeItem.h"
#include "src/ui/layout/base/MapLayoutItemDelegate.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"

#include <QTreeWidget>

class TreeMapHint : public AlterationAcknoledger, public AtomsContextualMenuHandler {
    
    Q_OBJECT
    
    public:
        TreeMapHint(AtomsStorage* mapMaster);

    public slots:
        void propagateFocus(snowflake_uid focusedAtomId);
        void propagateSelection(QVector<snowflake_uid> &selectedIds);
        void removeLayerItem(int layer);

    signals:
        void requestingItemInsertion(QTreeWidgetItem* item, QTreeWidgetItem* parent);
        void requestingItemClearing();
        void requestingItemDeletion(QTreeWidgetItem* toRemove);
        void requestingItemMove(QTreeWidgetItem* oldLayerItem, QTreeWidgetItem* newLayerItem, QTreeWidgetItem *item);
        void requestingItemTextChange(QTreeWidgetItem* toChange, const QString &newName);
        void requestingItemSelectionClearing();
        void requestingItemSelection(QTreeWidgetItem* toSelect);
        void requestingItemDataUpdate(QTreeWidgetItem* target, int column, const QHash<int, QVariant> &newData);

    private slots:
        void _onRenamedAsset(const RPZAssetHash &id, const QString &newName);

    private:
        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);
        void _bindOwnerToItem(QTreeWidgetItem* item, RPZUser &owner);

        QHash<snowflake_uid, QTreeWidgetItem*> _atomTreeItemsById;
        QHash<RPZAssetHash, QSet<snowflake_uid>> _atomIdsBoundByRPZAssetHash;

        //augmenting AtomsStorage
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;
        RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration);

        //icons
        QIcon* _layerIcon = nullptr;
        QIcon* _textIcon = nullptr;
        QIcon* _drawingIcon = nullptr;
};