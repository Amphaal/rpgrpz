#pragma once

#include <QAbstractItemModel>
#include <QMimeData>

#include <QPixmapCache>

#include <QIcon>
#include <QLabel>
#include <QString>

#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/shared/database/AssetsDatabase.h"

#include "src\shared\async-ui\AlterationHandler.h"
#include "src/ui/others/ClientBindable.h"

class AssetsTreeViewModel : public QAbstractItemModel, public ClientBindable {
    
    Q_OBJECT
    
    public:
        AssetsTreeViewModel(QObject *parent = nullptr);
        AssetsDatabase* database();
        void onRPZClientConnecting() override;

        ///////////////
        /// HELPERS ///
        ///////////////

        QPixmap getAssetIcon(AssetsDatabaseElement* target, QSize &sizeToApply) const;
        bool createFolder(QModelIndex &parentIndex);
        bool moveItems(const QMimeData *data, const QModelIndex &parentIndex);
        bool insertAssets(QList<QUrl> &urls, const QModelIndex &parentIndex);
        bool removeItems(const QList<QModelIndex> &itemsIndexesToRemove);
        
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

    private slots:
        void _onReceivedAsset(const RPZAssetImportPackage &package);

    private:
        
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
};