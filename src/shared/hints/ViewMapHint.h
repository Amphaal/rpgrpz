#pragma once

#include <QMultiHash>

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QMessageBox>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/shared/models/RPZToyMetadata.h"

#include "src/ui/map/graphics/CustomGraphicsItemHelper.h"

#include "src/shared/payloads/Payloads.h"
#include "src/ui/assets/base/AssetsTreeViewModel.h"

#include "src/shared/database/MapDatabase.h"

#include "AtomsStorage.h"

#include "src/ui/map/MapTools.h"

class ViewMapHint : public AtomsStorage {

    Q_OBJECT

    public:
        ViewMapHint();

        //might be called by another thread, safe
        RPZAtomId integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem) const;
        const RPZAtom templateAtom() const;
        QGraphicsItem* ghostItem() const;

        const QVector<RPZAtomId> getAtomIdsFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const; //safe
        const RPZAtomId getAtomIdFromGraphicsItem(const QGraphicsItem* toFetch) const; 

        QGraphicsItem* generateTemporaryItemFromTemplateBuffer(); //safe

    public slots:
        void mightNotifyMovement(const QList<QGraphicsItem*> &itemsWhoMightHaveMoved); //safe
        void notifySelectedItems(const QList<QGraphicsItem*> &selectedItems); //safe
        void notifyFocusedItem(QGraphicsItem* focusedItem); //safe
        void setDefaultUser(const RPZUser &user); //safe
        void setDefaultLayer(int layer); //safe

        //handle preview alteration before real payload
        void handlePreviewRequest(const AtomsSelectionDescriptor &selectionDescriptor, const AtomParameter &parameter, const QVariant &value);

    signals:
        void requestingUIAlteration(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter);
        void requestingUIUpdate(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate);
        void requestingUIUpdate(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates);
        void requestingUIUserChange(const QList<QGraphicsItem*> &toUpdate, const RPZUser &newUser);

    protected:
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;

    private:
        //alter template Atom
        mutable QMutex _m_ghostItem;
        QGraphicsItem* _ghostItem = nullptr;
        
        mutable QMutex _m_templateAsset;
        RPZToyMetadata _templateAsset;

        mutable QMutex _m_templateAtom;
        RPZAtom _templateAtom;
        
        mutable QMutex _m_GItemsByRPZAtomId;
        QMap<RPZAtomId, QGraphicsItem*> _GItemsByRPZAtomId;

        QMultiHash<RPZAssetHash, QGraphicsItem*> _missingAssetsIdsFromDb;
        
        //helpers
        QGraphicsItem* _generateGhostItem(const RPZToyMetadata &assetMetadata);
        QGraphicsItem* _buildGraphicsItemFromAtom(const RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(RPZAtom* atom, QGraphicsItem* gi);

        //missing assets tracking
        void _replaceMissingAssetPlaceholders(const RPZToyMetadata &metadata); //safe

        //augmenting AtomsStorage
        virtual RPZAtom* _insertAtom(const RPZAtom &newAtom) override;
        virtual RPZAtom* _changeOwner(RPZAtom* atomWithNewOwner, const RPZUser &newOwner) override;

        virtual void _basicAlterationDone(const QList<RPZAtomId> &updatedIds, const PayloadAlteration &type) override;
        virtual void _updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) override;
        virtual void _updatesDone(const AtomsUpdates &updates) override;
        virtual void _ownerChangeDone(const QList<RPZAtomId> &updatedIds, const RPZUser &newUser) override;

};