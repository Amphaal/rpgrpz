#pragma once

#include <QAbstractItemModel>
#include <QMimeData>

#include <QPixmapCache>

#include <QIcon>
#include <QLabel>
#include <QString>

#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/async-ui/AlterationActor.hpp"
#include "src/ui/others/ClientBindable.h"

class AssetsTreeViewModel : public QAbstractItemModel {
    
    Q_OBJECT
    
    public:
        AssetsTreeViewModel(QObject *parent = nullptr);
        
        QModelIndexList getPersistentIndexList() const;
        AssetsDatabase* database() const;

        ///////////////
        /// HELPERS ///
        ///////////////

        QPixmap getAssetIcon(AssetsDatabaseElement* target, QSize &sizeToApply) const;
        bool createFolder(QModelIndex &parentIndex);
        bool moveItems(const QMimeData *data, const QModelIndex &parentIndex);
        bool insertAssets(QList<QUrl> &urls, const QModelIndex &parentIndex);
        bool removeItems(const QList<QModelIndex> &itemsIndexesToRemove);
        void integrateAsset(const RPZAssetImportPackage &package);
        
        ///////////////////
        /// END HELPERS ///
        ///////////////////


        ////////////////////////
        /// REIMPLEMENTATION ///
        ////////////////////////

        //index
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

        //parent
        QModelIndex parent(const QModelIndex &index) const override;

        //flags
        Qt::ItemFlags flags(const QModelIndex &index) const override;

        //data
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        ////////////////////////////
        /// END REIMPLEMENTATION ///
        ////////////////////////////

    protected:
        AssetsDatabase* _db = nullptr;

        /////////////////////
        /// DROP HANDLING ///
        /////////////////////

        static QList<AssetsDatabaseElement*> pointerListFromMimeData(const QMimeData *data);

        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
        bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
        QMimeData* mimeData(const QModelIndexList &indexes) const override;
        Qt::DropActions supportedDropActions() const override;

        /////////////////////////
        /// END DROP HANDLING ///
        /////////////////////////

        QModelIndex _getDownloadableFolderIndex(); 

        mutable QModelIndexList _bufferedDraggedIndexes;
        QModelIndexList _getTopMostIndexesFromDraggedIndexesBuffer();
        bool _bufferContainsIndexOrParent(const QModelIndex &index);
        QPair<int, int> _anticipateInserts(const QModelIndexList &tbi);

};