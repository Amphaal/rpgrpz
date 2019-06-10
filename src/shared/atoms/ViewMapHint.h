#pragma once

#include <QMultiHash>

#include <QGraphicsView>
#include <QWidget>

#include <QFileInfo>

#include <QMessageBox>

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QPixmap>

#include "src/shared/models/Payloads.h"
#include "src/ui/assets/base/AssetsTreeViewModel.h"

#include "src/shared/database/MapDatabase.hpp"

#include "src/ui/map/graphics/MapViewGraphicsScene.hpp"

#include "AtomsStorage.h"

#include "src/ui/map/MapTools.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"

class ViewMapHint : public AtomsStorage, public AtomsContextualMenuHandler {
    
    Q_OBJECT

    public:
        ViewMapHint(QGraphicsView* boundGv);
        MapViewGraphicsScene* scene();
        bool isInTextInteractiveMode();

        //load/unload
        QString stateFilePath();
        bool loadDefaultState();
        bool loadState(QString &filePath);
        bool saveState();
        bool saveStateAs(QString &newFilePath);
        
        bool isDirty();
        void mayWantToSavePendingState();

        bool isRemote();
        bool defineAsRemote(QString &remoteMapDescriptor = QString());

        //replace placeholders
        void replaceMissingAssetPlaceholders(const QString &assetId);

        //actions helpers
        void deleteCurrentSelectionItems();
        void addDrawing(QGraphicsPathItem* drawn);

        //ghost handling
        QGraphicsItem* generateGhostItem(const AtomType &type, const QString assetLocation = QString());
        void turnGhostItemIntoDefinitive(QGraphicsItem* ghostItem);
        void centerGraphicsItemToPoint(QGraphicsItem* item, const QPoint &eventPos);

        //on move
        void handleAnyMovedItems();

        //pen
        QPen getPen() const;
        void setPenColor(QColor &color);
        void setPenSize(int size);

        //layer
        void setDefaultLayer(int layer);

    signals:
        void mapFileStateChanged(const QString &filePath, bool isDirty);
        void requestMissingAsset(const QString &assetIdToRequest);
        void selectionChanged(QVector<void*> &atoms);

    private:
        QGraphicsView* _boundGv = nullptr;
        
        //map state handling
        QString _stateFilePath;
        bool _isRemote = false;
        bool _isDirty = false;
        void _setDirty(bool dirty = true);
        void _shouldMakeDirty(AlterationPayload* payload);

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

        //missing assets tracking
        QMultiHash<QString, QGraphicsRectItem*> _missingAssetsIdsFromDb;

        //text interactive
        bool _isInTextInteractiveMode = false;

        //augmenting AtomsStorage
        virtual void _handlePayload(AlterationPayload* payload) override;
        virtual RPZAtom* _handlePayloadInternal(const PayloadAlteration &type, const snowflake_uid &targetedAtomId, QVariant &atomAlteration) override;

        //template
            QColor _penColor = Qt::blue;
            RPZAtom* _templateAtom = nullptr;
};