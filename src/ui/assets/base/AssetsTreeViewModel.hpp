#pragma once

#include <QAbstractItemModel>
#include <QMimeData>

#include <QIcon>
#include <QLabel>

#include "src/shared/assets/AssetsDatabaseElement.hpp"
#include "src/shared/assets/AssetsDatabase.h"

class AssetsTreeViewModel : public QAbstractItemModel {
    public:
        AssetsTreeViewModel(QObject *parent = nullptr) : QAbstractItemModel(parent), _db(new AssetsDatabase) { };
        
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
            auto data = static_cast<AssetsDatabaseElement*>(parent.internalPointer());
            
            return this->createIndex(row, column, data->child(row)); 
            
        }

        //parent
        QModelIndex parent(const QModelIndex &index) const override {
            
            //prepare data
            auto data = static_cast<AssetsDatabaseElement*>(index.internalPointer());

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

            return static_cast<AssetsDatabaseElement*>(index.internalPointer())->flags();
        }

        //data
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            
            //bound data...
            auto data = static_cast<AssetsDatabaseElement*>(index.internalPointer());

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

        int columnCount(const QModelIndex &parent = QModelIndex()) const override {
            return 2;
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const override {

            //is root
            if(!parent.isValid()) {
                return 1;
            }

            //is inner element
            auto data = static_cast<AssetsDatabaseElement*>(parent.internalPointer());
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

        bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override {
            return QAbstractItemModel::canDropMimeData(data, action, row, column, parent);
        }
        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override {
            return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
        }
        QStringList mimeTypes() const override {
            return QAbstractItemModel::mimeTypes();
        }
        QMimeData* mimeData(const QModelIndexList &indexes) const override {
            return QAbstractItemModel::mimeData(indexes);
        }
        Qt::DropActions supportedDropActions() const {
            return (
                Qt::DropAction::MoveAction | Qt::DropAction::CopyAction
            );
        }

        /////////////////////////
        /// END DROP HANDLING ///
        /////////////////////////

};