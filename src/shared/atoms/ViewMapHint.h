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
#include "src/shared/models/AssetMetadata.h"

class ViewMapHint : public AtomsStorage, public AtomsContextualMenuHandler {
    
    Q_OBJECT

    public:
        ViewMapHint(QGraphicsView* boundGv);
        MapViewGraphicsScene* scene();

        //replace placeholders
        void replaceMissingAssetPlaceholders(const RPZAssetId &id);

        //actions helpers
        void deleteCurrentSelectionItems();

        //ghost handling
        QGraphicsItem* generateGhostItem(AssetMetadata &assetMetadata);
        void integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem);

        //on move
        void handleAnyMovedItems();
        
        //special handling
        void handleParametersUpdateAlterationRequest(QVariantHash &payload);

        //alter template Atom
        RPZAtom* templateAtom = nullptr;
        void setDefaultUser(RPZUser user);
        void setDefaultLayer(int layer);

        QVector<RPZAtom*> selectedAtoms();

    signals:
        void mapFileStateChanged(const QString &filePath, bool isDirty);
        void requestMissingAssets(QList<RPZAssetId> assetIdsToRequest);
        void atomTemplateChanged();

    private:
        //helpers
        QGraphicsItem* _buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi);
        RPZAtom* _fetchAtom(QGraphicsItem* graphicElem) const;
        QVector<RPZAtom*> _fetchAtoms(const QList<QGraphicsItem*> &listToFetch) const;
        QVector<snowflake_uid> _selectedAtomIds() override;

        //inner event handling
        bool _preventInnerGIEventsHandling = false;
        void _onSceneSelectionChanged();
        void _onSceneItemChanged(QGraphicsItem* item, int changeFlag);
            QSet<QGraphicsItem*> _itemsWhoNotifiedMovement;

    protected:
        QGraphicsView* _boundGv = nullptr;

        //missing assets tracking
        QMultiHash<RPZAssetId, QGraphicsRectItem*> _missingAssetsIdsFromDb;
        QSet<RPZAssetId> _assetsIdsToRequest;

        //augmenting AtomsStorage
        virtual void _handlePayload(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) override;
};