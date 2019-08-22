#pragma once

#include "src\shared\async-ui\AlterationAcknoledger.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/ui/layout/base/LayerTreeItem.h"
#include "src/ui/layout/base/MapLayoutItemDelegate.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"

#include <QTreeWidget>
#include <QHash>

class TreeMapHint : public QObject, public AlterationAcknoledger {
    
    Q_OBJECT
    
    public:
        TreeMapHint();

    public slots:
        void propagateFocus(snowflake_uid focusedAtomId);
        void propagateSelection(QVector<snowflake_uid> &selectedIds);
        void removeLayerItem(int layer);
        void updateOwnerFromItem(QTreeWidgetItem* item, const RPZUser &owner);

    signals:
        void requestingUIAlteration(PayloadAlteration alteration, QList<QTreeWidgetItem*> &toAlter);
        void requestingUIUpdate(PayloadAlteration alteration, QHash<QTreeWidgetItem*, QHash<AtomParameter, QVariant>> &toUpdate);

    private slots:
        void _onRenamedAsset(const RPZAssetHash &id, const QString &newName);

    private:
        QHash<QTreeWidgetItem*, QHash<AtomParameter, QVariant>> _UIUpdatesBuffer;

        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

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