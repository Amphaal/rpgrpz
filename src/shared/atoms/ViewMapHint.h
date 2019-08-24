#pragma once

#include <QMultiHash>

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QMessageBox>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/ui/map/graphics/CustomGraphicsItemHelper.h"

#include "src/shared/payloads/Payloads.h"
#include "src/ui/assets/base/AssetsTreeViewModel.h"

#include "src/shared/database/MapDatabase.h"

#include "AtomsStorage.h"

#include "src/ui/map/MapTools.h"

#include "src/shared/models/RPZAssetMetadata.h"

class ViewMapHint : public QObject, public AtomsStorage {
    
    Q_OBJECT

    public:
        ViewMapHint();

        //actions helpers
        void deleteCurrentSelectionItems();

        //ghost handling
        QGraphicsItem* generateGhostItem(RPZAssetMetadata &assetMetadata);
        void integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem);
        
        //special handling
        void handleParametersUpdateAlterationRequest(QVariantHash &payload);
        void handlePreviewRequest(const QVector<snowflake_uid> &atomIdsToPreview, const AtomParameter &parameter, QVariant &value);

        //alter template Atom
        RPZAtom* templateAtom = nullptr;

        QVector<RPZAtom*> selectedAtoms();

    public slots:
        void notifyMovementOnItems(QList<QGraphicsItem*> &itemsWhoMoved);
        void notifySelectedItems(QList<QGraphicsItem*> &selectedItems);
        void replaceMissingAssetPlaceholders(const RPZAssetMetadata &metadata);
        void setDefaultUser(const RPZUser &user);
        void setDefaultLayer(int layer);

    signals:
        void requestMissingAssets(const QList<RPZAssetHash> &assetIdsToRequest);
        void atomTemplateChanged();

        void requestingUIAlteration(AlterationPayload &payload, QHash<snowflake_uid, QGraphicsItem*> &toUpdate);

        void heavyAlterationProcessing();

    private:
        QMap<snowflake_uid, QGraphicsItem*> _GItemsByAtomId;
        
        //helpers
        QGraphicsItem* _buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi);
        RPZAtom* _getAtomFromGraphicsItem(QGraphicsItem* graphicElem) const;
        QVector<RPZAtom*> _getAtomFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const;

    protected:
        QHash<snowflake_uid, QGraphicsItem*> _UIUpdatesBuffer;

        //missing assets tracking
        QMultiHash<RPZAssetHash, QGraphicsItem*> _missingAssetsIdsFromDb;
        QSet<RPZAssetHash> _assetsIdsToRequest;

        //augmenting AtomsStorage
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, snowflake_uid targetedAtomId, const QVariant &alteration) override;
};