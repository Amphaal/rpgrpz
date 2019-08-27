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

class ViewMapHint : public AtomsStorage {

    Q_OBJECT

    public:
        ViewMapHint();

        //might be called by another thread, safe
        void deleteCurrentSelectionItems() const;
        QGraphicsItem* generateGhostItem(const RPZAssetMetadata &assetMetadata);
        void integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem) const;
        RPZAtom* templateAtom() const;

    public slots:
        void notifyMovementOnItems(const QList<QGraphicsItem*> &itemsWhoMoved); //safe
        void notifySelectedItems(const QList<QGraphicsItem*> &selectedItems); //safe
        void replaceMissingAssetPlaceholders(const RPZAssetMetadata &metadata); //safe
        void setDefaultUser(const RPZUser &user); //safe
        void setDefaultLayer(int layer); //safe

        //handle Template update or standard piped alteration request
        void handleParametersUpdateAlterationRequest(AlterationPayload &payload);
        void handlePreviewRequest(const QVector<snowflake_uid> &atomIdsToPreview, const AtomParameter &parameter, QVariant &value);

    signals:
        void requestMissingAssets(const QList<RPZAssetHash> &assetIdsToRequest);
        void atomTemplateChanged();

        void requestingUIAlteration(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter);
        void requestingUIUpdate(const PayloadAlteration &type, const QHash<QGraphicsItem*, AtomUpdates> &toUpdate);
        void requestingUIUpdate(const PayloadAlteration &type, const QList<QGraphicsItem*> &toUpdate, AtomUpdates &updates);
        void requestingUIUserChange(const PayloadAlteration &type, const QList<QGraphicsItem*> &toUpdate, const RPZUser &newUser);

    protected:
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;

        QMultiHash<RPZAssetHash, QGraphicsItem*> _missingAssetsIdsFromDb;
        mutable QMutex _m_missingAssetsIdsFromDb;

    private:
        //alter template Atom
        RPZAtom* _templateAtom = nullptr;
        mutable QMutex _m_templateAtom;

        QMap<snowflake_uid, QGraphicsItem*> _GItemsByAtomId;
        
        //helpers
        QGraphicsItem* _buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi);
        RPZAtom* _getAtomFromGraphicsItem(QGraphicsItem* graphicElem) const;
        QVector<RPZAtom*> _getAtomFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const;

        //missing assets tracking
        QSet<RPZAssetHash> _assetsIdsToRequest;

        //augmenting AtomsStorage
        virtual RPZAtom* _insertAtom(const RPZAtom &newAtom) override;
        virtual RPZAtom* _changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner) override;
};