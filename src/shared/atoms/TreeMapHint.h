#pragma once

#include "src\shared\async-ui\AlterationAcknoledger.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/ui/layout/base/LayerTreeItem.h"
#include "src/ui/layout/base/MapLayoutItemDelegate.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"

#include "src/shared/models/RPZAtom.h"

#include <QTreeWidget>
#include <QHash>

class TreeMapHint : public QObject, public AlterationAcknoledger {
    
    Q_OBJECT
    
    public:
        TreeMapHint();

    public slots:
        void propagateFocus(RPZAtomId focusedRPZAtomId);
        void propagateSelection(QVector<RPZAtomId> &selectedIds);
        void removeLayerItem(int layer);
        void updateOwnerFromItem(QTreeWidgetItem* item, const RPZUser &owner);

    signals:
        void requestingUIAlteration(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter);
        void requestingUIUpdate(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate);
        void requestingUIUpdate(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates);
        void requestingUIUserChange(const QList<QGraphicsItem*> &toUpdate, const RPZUser &newUser);

    private slots:
        void _onRenamedAsset(const RPZAssetHash &id, const QString &newName);

    private:
        QHash<int, QTreeWidgetItem*> _layersItems;
        QTreeWidgetItem* _getLayerItem(int layer);

        QTreeWidgetItem* _createTreeItem(RPZAtom &atom);

        QHash<RPZAtomId, QTreeWidgetItem*> _atomTreeItemsById;
        QHash<RPZAssetHash, QSet<RPZAtomId>> _RPZAtomIdsBoundByRPZAssetHash;

        //augmenting AtomsStorage
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;
        RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, RPZAtomId targetedRPZAtomId, const QVariant &alteration);

        //icons
        QIcon* _layerIcon = nullptr;
        QIcon* _textIcon = nullptr;
        QIcon* _drawingIcon = nullptr;
};