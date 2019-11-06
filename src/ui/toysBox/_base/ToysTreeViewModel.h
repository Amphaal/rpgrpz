#pragma once

#include <QAbstractItemModel>
#include <QMimeData>

#include <QPixmapCache>

#include <QIcon>
#include <QLabel>
#include <QString>

#include "ToysTreeViewItem.h"
#include "src/shared/database/AssetsDatabase.h"

#include "src/shared/async-ui/AlterationActor.hpp"

class ToysTreeViewModel : public QAbstractItemModel {
    
    Q_OBJECT
    
    public:
        ToysTreeViewModel(QObject *parent = nullptr);
        
        QModelIndex getStaticContainerTypesIndex(const ToysTreeViewItem::Type &staticContainerType); 

        ///////////////
        /// HELPERS ///
        ///////////////

        const QModelIndex createFolder(const QModelIndex &parentIndex);
        bool moveItemsToContainer(const QModelIndex &parentIndex, const QList<QModelIndex> &indexesToMove);
        bool insertAssets(const QList<QUrl> &urls, const QModelIndex &parentIndex);
        void removeItems(const QList<QModelIndex> &itemsIndexesToRemove);
        bool integrateAsset(RPZAssetImportPackage &package);
        bool renameItem(const QString &newName, const QModelIndex &index);
        
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
        /////////////////////
        /// DROP HANDLING ///
        /////////////////////

        static QList<ToysTreeViewItem*> fromMimeData(const QMimeData *data);

        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
        bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
        QMimeData* mimeData(const QModelIndexList &indexes) const override;
        Qt::DropActions supportedDropActions() const override;

        /////////////////////////
        /// END DROP HANDLING ///
        /////////////////////////

        mutable QModelIndexList _bufferedDraggedIndexes;
        QModelIndexList _getTopMostIndexes(const QModelIndexList &indexesList);
        bool _indexListContainsIndexOrParent(const QModelIndexList &base, const QModelIndex &index);
        QPair<int, int> _anticipateInserts(const QModelIndexList &tbi);

    private:
        ToysTreeViewItem* _rootItem = nullptr;
        QHash<ToysTreeViewItem::Type, ToysTreeViewItem*> _staticElements;  

        void _injectStaticStructure();
        void _injectDbStructure();              
            
            //returns last elem by path created
            QHash<AssetsDatabase::FolderPath, ToysTreeViewItem*> _generateFolderTreeFromDb();

            //iterate through paths chunks and create missing folders at each pass, returns last folder found/created
            ToysTreeViewItem* _recursiveElementCreator(ToysTreeViewItem* parent, QList<QString> &pathChunks); 

            //from definitive paths, fetch items from db and generate elements
            void _generateItemsFromDb(const QHash<AssetsDatabase::FolderPath, ToysTreeViewItem*> &pathsToFillWithItems);

};