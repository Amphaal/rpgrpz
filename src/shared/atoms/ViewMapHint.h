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
        const RPZAtom* templateAtom() const;

    public slots:
        void notifyMovementOnItems(const QList<QGraphicsItem*> &itemsWhoMoved); //safe
        void notifySelectedItems(const QList<QGraphicsItem*> &selectedItems); //safe
        void setDefaultUser(const RPZUser &user); //safe
        void setDefaultLayer(int layer); //safe

        //handle preview alteration before real payload
        void handlePreviewRequest(const QVector<RPZAtomId> &RPZAtomIdsToPreview, const AtomParameter &parameter, QVariant &value);

    signals:
        void requestMissingAssets(const QList<RPZAssetHash> &assetIdsToRequest);
        void atomTemplateChanged();

        void requestingUIAlteration(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter);
        void requestingUIUpdate(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate);
        void requestingUIUpdate(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates);
        void requestingUIUserChange(const QList<QGraphicsItem*> &toUpdate, const RPZUser &newUser);

    protected:
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;

        QMultiHash<RPZAssetHash, QGraphicsItem*> _missingAssetsIdsFromDb;
        mutable QMutex _m_missingAssetsIdsFromDb;

    private:
        //alter template Atom
        RPZAtom* _templateAtom = nullptr;
        mutable QMutex _m_templateAtom;

        QMap<RPZAtomId, QGraphicsItem*> _GItemsByRPZAtomId;
        
        //helpers
        QGraphicsItem* _buildGraphicsItemFromAtom(RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi);
        RPZAtom* _getAtomFromGraphicsItem(QGraphicsItem* graphicElem) const;
        QVector<RPZAtom*> _getAtomsFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const;

        //missing assets tracking
        QSet<RPZAssetHash> _assetsIdsToRequest;
        void _replaceMissingAssetPlaceholders(const RPZAssetMetadata &metadata); //safe

        //augmenting AtomsStorage
        virtual RPZAtom* _insertAtom(const RPZAtom &newAtom) override;
        virtual RPZAtom* _changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner) override;

        virtual void _basicAlterationDone(const QList<RPZAtomId> &updatedIds, const PayloadAlteration &type) override;
        virtual void _updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) override;
        virtual void _updatesDone(const AtomsUpdates &updates) override;
        virtual void _ownerChangeDone(const QList<RPZAtomId> &updatedIds, const RPZUser &newUser) override;
};