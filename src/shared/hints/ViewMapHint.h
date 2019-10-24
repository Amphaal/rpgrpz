#pragma once

#include <QMultiHash>

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QMessageBox>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/shared/models/toy/RPZAsset.hpp"

#include "src/ui/map/graphics/CustomGraphicsItemHelper.h"

#include "src/shared/payloads/Payloads.h"
#include "src/ui/assets/_base/AssetsTreeViewModel.h"

#include "AtomsStorage.h"

#include "src/helpers/RPZQVariant.hpp"

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
        void setDefaultLayer(int layer); //safe

        //handle preview alteration before real payload
        void handlePreviewRequest(const AtomsSelectionDescriptor &selectionDescriptor, const AtomParameter &parameter, const QVariant &value);

    signals:
        void requestingUIAlteration(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter);
        void requestingUIUpdate(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate);
        void requestingUIUpdate(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates);

    protected:
        virtual void _handleAlterationRequest(AlterationPayload &payload) override;

    private:
        //alter template Atom
        mutable QMutex _m_ghostItem;
        QGraphicsItem* _ghostItem = nullptr;
        
        mutable QMutex _m_templateAsset;
        RPZToy _templateToy;

        mutable QMutex _m_templateAtom;
        RPZAtom _templateAtom;
        
        mutable QMutex _m_GItemsByRPZAtomId;
        QMap<RPZAtomId, QGraphicsItem*> _GItemsByRPZAtomId;

        QMultiHash<RPZAssetHash, QGraphicsItem*> _missingAssetsIdsFromDb;
        
        //helpers
        QGraphicsItem* _generateGhostItem(const RPZToy &toy);
        QGraphicsItem* _buildGraphicsItemFromAtom(const RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(const RPZAtom &atom, QGraphicsItem* gi);

        //missing assets tracking
        void _replaceMissingAssetPlaceholders(const RPZAsset &metadata); //safe

        //augmenting AtomsStorage
        void addAtom(const RPZAtom &toAdd) override;

        virtual void _basicAlterationDone(const QList<RPZAtomId> &updatedIds, const PayloadAlteration &type) override;
        virtual void _updatesDone(const QList<RPZAtomId> &updatedIds, const AtomUpdates &updates) override;
        virtual void _updatesDone(const AtomsUpdates &updates) override;

};