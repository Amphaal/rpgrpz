#pragma once

#include <QMultiHash>

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QMessageBox>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/shared/payloads/Payloads.h"
#include "src/ui/assets/base/AssetsTreeViewModel.h"

#include "src/shared/database/MapDatabase.h"

#include "src/ui/map/graphics/MapViewGraphicsScene.h"

#include "AtomsStorage.h"

#include "src/ui/map/MapTools.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/models/RPZAssetMetadata.h"

class ViewMapHint : public AtomsStorage, public AtomsContextualMenuHandler {
    
    Q_OBJECT

    public:
        ViewMapHint();

        //replace placeholders
        void replaceMissingAssetPlaceholders(const RPZAssetMetadata &metadata);

        //actions helpers
        void deleteCurrentSelectionItems();

        //ghost handling
        QGraphicsItem* generateGhostItem(RPZAssetMetadata &assetMetadata);
        void integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem);

        //on move
        void handleAnyMovedItems();
        
        //special handling
        void handleParametersUpdateAlterationRequest(QVariantHash &payload);
        void handlePreviewRequest(const QVector<snowflake_uid> &atomIdsToPreview, const AtomParameter &parameter, QVariant &value);

        //alter template Atom
        RPZAtom* templateAtom = nullptr;
        QGraphicsItem* templateGItem = nullptr;

        void setDefaultUser(RPZUser user);
        void setDefaultLayer(int layer);

        QVector<RPZAtom*> selectedAtoms();

    signals:
        void mapFileStateChanged(const QString &filePath, bool isMapDirty);
        void requestMissingAssets(const QList<RPZAssetHash> assetIdsToRequest);
        void atomTemplateChanged();

        void requestingItemInsertion(QGraphicsItem* graphicElem);
        void requestingAllItemsRemoval();

    private:
        //helpers
        QGraphicsItem* _buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi);
        RPZAtom* _getAtomFromGraphicsItem(QGraphicsItem* graphicElem) const;
        QVector<RPZAtom*> _getAtomFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const;

        RPZMap<QGraphicsItem*> _GItemsByAtomId;

        //inner event handling
        bool _preventInnerGIEventsHandling = false;
        void _onSceneSelectionChanged();
        void _onSceneItemChanged(QGraphicsItem* item, int changeFlag);
            QSet<QGraphicsItem*> _itemsWhoNotifiedMovement;

    protected:
        //missing assets tracking
        QMultiHash<RPZAssetHash, QGraphicsRectItem*> _missingAssetsIdsFromDb;
        QSet<RPZAssetHash> _assetsIdsToRequest;

        //augmenting AtomsStorage
        virtual void _handlePayload(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) override;
};