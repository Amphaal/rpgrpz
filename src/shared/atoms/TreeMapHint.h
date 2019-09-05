#pragma once

#include "src/shared/async-ui/AlterationAcknoledger.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/ui/layout/base/LayerTreeItem.h"
#include "src/ui/layout/base/MapLayoutItemDelegate.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"

#include "src/shared/models/RPZAtom.h"

#include <QTreeWidget>
#include <QHash>

struct LayerManipulationHelper {
    QHash<QTreeWidgetItem*, int> toRemoveChildrenCountByLayerItem;
    QHash<int, QList<QTreeWidgetItem*>> childrenMovedToLayer;
};

class TreeMapHint : public AlterationAcknoledger {
    
    Q_OBJECT
    
    public:
        TreeMapHint();

        QTreeWidgetItem* getLayerItem(int layer) const; //safe
        void updateOwnerFromItem(QTreeWidgetItem* item, const RPZUser &owner); //safe
        void propagateFocus(RPZAtomId focusedRPZAtomId); //safe
        void propagateSelection(QVector<RPZAtomId> &selectedIds); //safe

    signals:
        void requestingUIAlteration(const PayloadAlteration &type, const QList<QTreeWidgetItem*> &toAlter);
        void requestingUIUpdate(const QHash<QTreeWidgetItem*, AtomUpdates> &toUpdate);
        void requestingUIUpdate(const QList<QTreeWidgetItem*> &toUpdate, const AtomUpdates &updates);
        void requestingUIUserChange(const QList<QTreeWidgetItem*> &toUpdate, const RPZUser &newUser);
        void requestingUIMove(const QHash<int, QList<QTreeWidgetItem*>> &childrenMovedToLayer);

    private slots:
        void _onRenamedAsset(const RPZAssetHash &id, const QString &newName);

    private:
        mutable QMutex _m_layersItems;
        QHash<int, QTreeWidgetItem*> _layersItems;
        void _mayCreateLayerItem(int layer);

        QTreeWidgetItem* _createTreeItem(const RPZAtom &atom);

        QHash<RPZAtomId, QTreeWidgetItem*> _atomTreeItemsById;
        QHash<RPZAssetHash, QSet<RPZAtomId>> _RPZAtomIdsBoundByRPZAssetHash;

        //handling
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;
            LayerManipulationHelper _mvHelper;
            void _handleItemMove(QTreeWidgetItem* toUpdate, const AtomUpdates &updatesMightContainMove);
        

        //icons
        QIcon* _layerIcon = nullptr;
        QIcon* _textIcon = nullptr;
        QIcon* _drawingIcon = nullptr;

        
};