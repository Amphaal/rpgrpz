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

#include "src/shared/renderer/AtomRenderer.h"

#include "src/shared/payloads/Payloads.h"
#include "src/ui/toys/_base/ToysTreeViewModel.h"

#include "AtomsStorage.h"

#include "src/helpers/RPZQVariant.hpp"

class ViewMapHint : public AtomsStorage {

    Q_OBJECT

    public:
        ViewMapHint(const QSizeF &defaultTileSize);

        //might be called by another thread, safe
        RPZAtom::Id integrateGraphicsItemAsPayload(QGraphicsItem* ghostItem) const;
        const RPZAtom templateAtom() const;
        QGraphicsItem* ghostItem() const;

        const QList<RPZAtom::Id> getAtomIdsFromGraphicsItems(const QList<QGraphicsItem*> &listToFetch) const; //safe
        const RPZAtom::Id getAtomIdFromGraphicsItem(const QGraphicsItem* toFetch) const; 

        QGraphicsItem* generateTemporaryItemFromTemplateBuffer(); //safe
        const QSizeF standardTileSize() const;

    public slots:
        void mightNotifyMovement(const QList<QGraphicsItem*> &itemsWhoMightHaveMoved); //safe
        void notifySelectedItems(const QList<QGraphicsItem*> &selectedItems); //safe
        void notifyFocusedItem(QGraphicsItem* focusedItem); //safe
        void setDefaultLayer(int layer); //safe

        //handle preview alteration before real payload
        void handlePreviewRequest(const AtomsSelectionDescriptor &selectionDescriptor, const RPZAtom::Parameter &parameter, const QVariant &value);

    signals:
        void requestingUIAlteration(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter);
        void requestingUIUpdate(const QHash<QGraphicsItem*, RPZAtom::Updates> &toUpdate);
        void requestingUIUpdate(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates);

    protected:
        virtual void _handleAlterationRequest(const AlterationPayload &payload) override;

    private:
        //alter template Atom
        mutable QMutex _m_ghostItem;
        QGraphicsItem* _ghostItem = nullptr;
        QSizeF _defaultTileSize;
        
        mutable QMutex _m_templateToy;
        RPZToy _templateToy;

        mutable QMutex _m_templateAtom;
        RPZAtom _templateAtom;
        
        mutable QMutex _m_GItemsByRPZAtomId;
        QMap<RPZAtom::Id, QGraphicsItem*> _GItemsByRPZAtomId;

        QMultiHash<RPZAsset::Hash, QGraphicsItem*> _missingAssetHashesFromDb;
        
        //helpers
        QGraphicsItem* _generateGhostItem(const RPZToy &toy);
        QGraphicsItem* _buildGraphicsItemFromAtom(const RPZAtom &atomToBuildFrom);
        void _crossBindingAtomWithGI(const RPZAtom &atom, QGraphicsItem* gi);

        //missing assets tracking
        void _replaceMissingAssetPlaceholders(const RPZAsset &metadata); //safe

        //augmenting AtomsStorage
        void _atomAdded(const RPZAtom &added) override;

        virtual void _basicAlterationDone(const QList<RPZAtom::Id> &updatedIds, const Payload::Alteration &type) override;
        virtual void _updatesDone(const QList<RPZAtom::Id> &updatedIds, const RPZAtom::Updates &updates) override;
        virtual void _updatesDone(const RPZAtom::ManyUpdates &updates) override;

};