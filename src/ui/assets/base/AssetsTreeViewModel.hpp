#pragma once

#include <QAbstractItemModel>
#include <QMimeData>

#include <QIcon>
#include <QLabel>
#include <QString>

#include "src/shared/assets/AssetsDatabaseElement.h"
#include "src/shared/assets/AssetsDatabase.h"

class AssetsTreeViewModel : public QAbstractItemModel {
    public:
        AssetsTreeViewModel(QObject *parent = nullptr) : QAbstractItemModel(parent), _db(new AssetsDatabase) { };

        ///////////////
        /// HELPERS ///
        ///////////////

        bool createFolder(QModelIndex &parentIndex) {
            this->beginInsertRows(parentIndex, 0, 1);
            this->_db->createFolder(
                AssetsDatabaseElement::fromIndex(parentIndex)
            );
            this->endInsertRows();
        }


        bool removeItems(const QList<QModelIndex> &itemsIndexesToRemove) {
            
            //create list
            QList<AssetsDatabaseElement*> items;
            for(auto &item : itemsIndexesToRemove) {
                items.append(
                    AssetsDatabaseElement::fromIndex(item)
                );
            }
            
            //TODO being/end
            this->_db->removeItems(items);
            
        }
        
        ///////////////////
        /// END HELPERS ///
        ///////////////////


        ////////////////////////
        /// REIMPLEMENTATION ///
        ////////////////////////

        //index
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override {
            
            //if root element requested..
            if(!parent.isValid()) {
                return this->createIndex(row, column, (AssetsDatabaseElement*)this->_db);
            }
            
            //prepare data
            auto data = AssetsDatabaseElement::fromIndex(parent);
            
            return this->createIndex(row, column, data->child(row)); 
            
        }

        //parent
        QModelIndex parent(const QModelIndex &index) const override {
            
            //prepare data
            auto data = AssetsDatabaseElement::fromIndex(index);

            //if root element requested..
            if(data->isRoot()) {
                return QModelIndex();
            }

            auto parent = data->parent();
            return this->createIndex(parent->row(), 0, parent);
        }

        //flags
        Qt::ItemFlags flags(const QModelIndex &index) const override {

            if(!index.isValid()) {
                return 0;
            }

            return AssetsDatabaseElement::fromIndex(index)->flags();
        }

        //data
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            
            //bound data...
            auto data = AssetsDatabaseElement::fromIndex(index);

            //for handled roles
            switch (role) {
                case Qt::DisplayRole:
                    switch(index.column()) {
                        case 0:
                            return data->displayName();
                        case 1:
                            return data->isContainer() ? QString::number(data->childCount()) : QVariant();
                    }
                case Qt::DecorationRole:
                    return index.column() == 0 ? QIcon(data->iconPath()) : QVariant();
                default:
                    return QVariant();
            }
            
        }

        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
            
            //bound data...
            auto data = AssetsDatabaseElement::fromIndex(index);

            //if not main column, default behavior
            if(index.column()) return QAbstractItemModel::setData(index, value, role);

            switch(role) {
                case Qt::EditRole:
                    return this->_db->rename(value.toString(), data);
                    break;
                default:
                    return QAbstractItemModel::setData(index, value, role);
            }
        }

        int columnCount(const QModelIndex &parent = QModelIndex()) const override {
            return 2;
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const override {

            //is root
            if(!parent.isValid()) {
                return 1;
            }

            //is inner element
            auto data = AssetsDatabaseElement::fromIndex(parent);
            return data->childCount();
        }

        ////////////////////////////
        /// END REIMPLEMENTATION ///
        ////////////////////////////

    protected:
        AssetsDatabase* _db = nullptr;

    private:
        
        /////////////////////
        /// DROP HANDLING ///
        /////////////////////

        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override {
            
            //if no handled format...
            if(!data->hasFormat("text/uri-list") && !data->hasFormat(AssetsDatabaseElement::listMimeType)) return false;

            auto droppedList = pointerListFromMimeData(data);
            auto dest = AssetsDatabaseElement::fromIndex(parent);

            //inner drop, move
            if(data->hasFormat(AssetsDatabaseElement::listMimeType)) {
                //TODO begin/end
                this->_db->moveItems(droppedList, dest);
            }

            //external drop, insert 
            if(data->hasFormat("text/uri-list")){
                auto urls = data->urls();

                //for each url, insert
                this->beginInsertRows(parent, 0, urls.count());
                for(auto &url : urls) {
                    this->_db->insertAsset(url, dest);
                }
                this->endInsertRows();
            }

            return true;
        }

        static QList<AssetsDatabaseElement*> pointerListFromMimeData(const QMimeData *data) {
            //get bound data
            auto asString = QString::fromUtf8(
                data->data(AssetsDatabaseElement::listMimeType)
            );
            auto pointerList = asString.split(";");

            //iterate through list
            QList<AssetsDatabaseElement*> list;
            for(auto &pointerAsString : pointerList) {
                auto ptr = (AssetsDatabaseElement*)pointerAsString.toLongLong();
                list << ptr;
            }

            return list;
        }

        bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override {
            
            //external drop, allows
            if(!data->hasFormat(AssetsDatabaseElement::listMimeType)) return true;

            //iterate through list
            bool hasInternal = false;
            QSet<AssetsDatabaseElement::Type> staticContainersSources;
            for(auto &ptr : pointerListFromMimeData(data)) {
                if(ptr->isInternal()) hasInternal = true;
                staticContainersSources.insert(ptr->getBoundStaticContainer());
            }

            //internal drop/move, reject internal objects drop
            if(hasInternal) return false;

            //if multiples source static containers types found, reject 
            if(staticContainersSources.count() > 1) return false;

            //if drop is not in source static container, reject
            auto destStaticContainerType = AssetsDatabaseElement::fromIndex(parent)->getBoundStaticContainer();
            if(staticContainersSources.contains(destStaticContainerType)) return false;

            return true;
        }

        QMimeData* mimeData(const QModelIndexList &indexes) const override {
            
            //create pointer list
            QString pointerList;
            for(auto &i : indexes) {

                //only first column
                if(i.column() > 0) continue;

                //add to list
                pointerList += QString::number((long long)i.internalPointer()) + ";";
            }
            pointerList = pointerList.left(pointerList.length() - 1); //remove last separator


            //mime
            auto mimeData = new QMimeData;
            mimeData->setData(AssetsDatabaseElement::listMimeType, pointerList.toUtf8());
            return mimeData;
        }

        Qt::DropActions supportedDropActions() const {
            return (
                Qt::DropAction::CopyAction | Qt::DropAction::MoveAction
            );
        }

        /////////////////////////
        /// END DROP HANDLING ///
        /////////////////////////
};