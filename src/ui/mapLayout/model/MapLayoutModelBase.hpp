// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QAbstractItemModel>
#include <QHash>

#include "src/shared/models/RPZAtom.h"

#include "MapLayoutAtom.h"
#include "MapLayoutCategory.h"

class MapLayoutModelBase : public QAbstractItemModel {
    public:
        MapLayoutModelBase() {}

        static const QList<RPZAtom::Id> fromIndexes(const QModelIndexList &selectedIndexes) {
            
            QList<RPZAtom::Id> selectedIds;
            
            for(const auto &index : selectedIndexes) {
                if(index.column()) continue;
                auto id = fromIndex(index);
                if(id) selectedIds += id;
            }

            return selectedIds;

        }

        static RPZAtom::Id fromIndex(const QModelIndex &index) {
            
            auto item = MapLayoutItem::fromIndex(index);

            auto atom = dynamic_cast<MapLayoutAtom*>(item);
            if(!atom) return 0;
            
            return atom->atomId();

        }

        const QModelIndex toIndex(const RPZAtom::Id &id) {
            
            auto atom = this->getMapLayoutAtom(id);
            auto category = atom->parent();

            auto categoryRow = this->_getRow(category);
            auto atomRow = category->rowOfAtom(atom);

            auto categoryIndex = this->index(categoryRow, 0);
            auto atomIndex = this->index(atomRow, 0, categoryIndex);

            return atomIndex;
        }

        const QModelIndexList toIndexes(const QList<RPZAtom::Id> &ids) {
            QModelIndexList out;
            for(const auto &id : ids) {
                out += this->toIndex(id);
            }
            return out;
        }

        //
        //
        //

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override {
            
            //if parent is not valid (eg Root), requires a category
            if(!parent.isValid()) {

                //if category is null, means nothing has been loaded
                auto category = this->_getCategory(row);
                if(!category) return QModelIndex();

                return this->createIndex(row, column, category);

            }

            auto base = MapLayoutItem::fromIndex(parent);
            if(!base) {
                qDebug() << "shouldnt";
                return QModelIndex();
            }
            
            //category, requires atom
            if(auto category = dynamic_cast<MapLayoutCategory*>(base)) {
                auto atom = category->atom(row);
                return this->createIndex(row, column, atom);
            } 

            qDebug() << "shouldnt";
            return QModelIndex();
    
        }

        QModelIndex parent(const QModelIndex &child) const override {
            
            //root, have no parent
            if(!child.isValid()) return QModelIndex();

            auto base = MapLayoutItem::fromIndex(child);
            if(!base) {
                qDebug() << "shouldnt";
                return QModelIndex();
            }
            
            //category, root is parent
            if(auto category = dynamic_cast<MapLayoutCategory*>(base)) {
                return QModelIndex();
            } 
            
            //atom, category is parent
            else if(auto atom = dynamic_cast<MapLayoutAtom*>(base)) {
                auto category = atom->parent();
                return this->createIndex(this->_getRow(category), 0, category);
            }

            qDebug() << "shouldnt";
            return QModelIndex();

        }
    
        int rowCount(const QModelIndex &parent = QModelIndex()) const override {
            
            //from root, count categories to display
            if(!parent.isValid()) {
                return this->countCategories();
            }
            
            //else, can only be atom count
            auto category = MapLayoutCategory::fromIndex(parent);
            if(!category) {
                qDebug() << "shouldnt";
                return 0;
            }

            return category->atomsCount();


        }

        int columnCount(const QModelIndex &parent = QModelIndex()) const override {
             return 3;
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            
            if(!index.isValid()) return QVariant();
            
            auto data = MapLayoutItem::fromIndex(index);
            if(!data) return QVariant();

            //for handled roles
            switch (role) {
                
                case Qt::TextAlignmentRole: {
                    return index.column() == 2 ? Qt::AlignRight : QVariant();
                }
                break;
                
                case Qt::DisplayRole: {
                    switch(index.column()) {
                        case 0:
                            return data->name();
                        case 2: {
                            if(auto category = dynamic_cast<MapLayoutCategory*>(data)) {
                                return QString::number(category->atomsCount());
                            }
                        }
                    }
                }
                break;

                case Qt::DecorationRole: {
                    
                    if(index.column()) return QVariant();

                    return data->icon();

                }
                break;

                default:
                    return QVariant();
            }


            return QVariant();

        }

        Qt::ItemFlags flags(const QModelIndex &index) const override {

            //if not first column
            if(index.column()) return 0;

            //root
            if(!index.isValid()) {
                return 0;
            }

            auto base = MapLayoutItem::fromIndex(index);
            if(!base) return 0;

            //category
            if(auto category = dynamic_cast<MapLayoutCategory*>(base)) {
                return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled);
            } 
            
            //atom
            else if(auto atom = dynamic_cast<MapLayoutAtom*>(base)) {
                return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable);
            }

            return 0;

        }

    protected:
        QHash<RPZAtom::Id, MapLayoutAtom*> _atomsByAtomId;
        QHash<RPZAsset::Hash, QSet<RPZAtom::Id>> _atomsByAssetHash;
        QMap<RPZAtom::Category, QMap<int, MapLayoutCategory*>> _categories;

        MapLayoutAtom* getMapLayoutAtom(const RPZAtom::Id &id) const {
            auto out = this->_atomsByAtomId.value(id);
            Q_ASSERT(out);
            return out;
        }

        void _createAtom(const RPZAtom &atom, bool triggerRowInsert = true) {
            
            //preapre
            auto categoryIndex = this->_mayCreateCategory(atom);
            auto category = MapLayoutCategory::fromIndex(categoryIndex);
            auto expectedRow = category->atomsCount();

            //start insert
            if(triggerRowInsert) this->beginInsertRows(categoryIndex, expectedRow, expectedRow);
                
                //add to tree
                auto mAtom = new MapLayoutAtom(category, atom);

                //add to catalog
                auto atomId = mAtom->atomId();
                this->_atomsByAtomId.insert(atomId, mAtom);
                
                //if has assetHash, add it
                auto assetHash = mAtom->assetHash();
                if(!assetHash.isNull()) {
                    this->_atomsByAssetHash[assetHash].insert(atomId);
                }

            if(triggerRowInsert) this->endInsertRows();

        }

        MapLayoutCategory* _removeAtom(const RPZAtom::Id &toRemove) {
            
            //prepare
            auto atom = this->getMapLayoutAtom(toRemove);
            auto category = atom->parent();
            auto categoryIndex = this->_getCategoryIndex(category);
            auto begin = category->rowOfAtom(atom);
            
            //start deletion
            this->beginRemoveRows(categoryIndex, begin, begin);

                //remove atom from catalog
                this->_atomsByAtomId.remove(toRemove);

                //if has assetHash, remove it from tracking list
                auto hash = atom->assetHash();
                if(!hash.isNull()) {
                    this->_atomsByAssetHash[hash].remove(toRemove);
                }

                //delete in tree
                delete atom;

            this->endRemoveRows();

            //update row count in category
            auto categoryCountIndex = categoryIndex.siblingAtColumn(2);
            emit dataChanged(categoryCountIndex, categoryCountIndex, {Qt::DisplayRole});

            return category;

        }

        
        void _removeCategory(MapLayoutCategory* toRemove) {
            
            //prepare
            auto rowToRemoveFrom = this->_getRow(toRemove);
            
            this->beginRemoveRows(QModelIndex(), rowToRemoveFrom, rowToRemoveFrom);
                
                this->_categories[toRemove->category()].remove(toRemove->sorter()); //remove from catalog
                delete toRemove; //remove from tree

            this->endRemoveRows();

        }

        MapLayoutCategory* _getCategory(int row) const {
            
            for(const auto &categoryItems : this->_categories) {
                
                auto count = categoryItems.count();
                
                if(row >= count) {
                    row = row - count;
                    continue;
                }

                auto sorter = categoryItems.keys().at(row);
                auto ptr = categoryItems.value(sorter);
                return ptr;

            }

            return nullptr;

        }

        int countCategories() const {
            auto total = 0;
            for(const auto &categoryItems : this->_categories) {
                total += categoryItems.count();
            }
            return total;
        }

        void _clearAll() {
            
            if(!this->_atomsByAtomId.count()) return;

            qDeleteAll(this->_atomsByAtomId);
            this->_atomsByAtomId.clear();    

            this->_atomsByAssetHash.clear();  

            for(const auto &category : this->_categories) {
                qDeleteAll(category);
            }
            this->_categories.clear();

        }

        QModelIndex _getCategoryIndex(const RPZAtom::Layer &layer, bool* created) {
            return this->_mayCreateCategory(RPZAtom::Category::Layout, layer, created);
        }

        QModelIndex _getCategoryIndex(MapLayoutCategory* category) {
            return this->index(this->_getRow(category), 0);
        }

    private:
        int _getRow(MapLayoutCategory* category) const {
            auto row = -1;
            
            for(const auto &categoryItems : this->_categories) {
                for(const auto &item : categoryItems) {
                    row++;
                    if(item == category) return row;
                }
            }

            return -1;
        }

        //anticipate row
        int _getRow(const RPZAtom::Category &category, const int &sorter) const {
            
            auto row = 0;

            for(auto i = this->_categories.begin(); i != this->_categories.end(); i++) {

                //if not yet the searched category
                if(i.key() != category) {
                    row += i.value().count();
                    continue;
                }

                auto keys = i.value().keys();

                //search for exact
                auto foundExact = keys.indexOf(sorter);
                if(foundExact > -1) return row + foundExact;

                //else, determine position
                keys.append(sorter);
                std::sort(keys.begin(), keys.end());
                auto foundInserted = keys.indexOf(sorter);
                if(foundInserted > -1) return row + (int)foundInserted;
               
            }
            
            
            return row;

        }

        QModelIndex _mayCreateCategory(const RPZAtom &atom) {
            
            QModelIndex out;
            
            auto category = atom.category();
            switch(category) {
                case RPZAtom::Category::Layout: {
                    out = this->_mayCreateCategory(category, atom.layer());
                }
                break;

                case RPZAtom::Category::Interactive: {
                    out = this->_mayCreateCategory(category, (int)atom.type());
                }
                break;

                default:
                break;
            }

            return out;

        }

        QModelIndex _mayCreateCategory(const RPZAtom::Category &category, const int &sorter, bool* created = nullptr) {
            
            auto begin = -1;
            auto categoryItem = this->_categories[category].value(sorter);
                
            if(!categoryItem) {
                
                begin = this->_getRow(category, sorter);
                this->beginInsertRows(QModelIndex(), begin, begin);

                    categoryItem = new MapLayoutCategory(category, sorter);
                    this->_categories[category].insert(sorter, categoryItem);
   
                this->endInsertRows();

                if(created) *created = true;

            } 
            
            else {
                begin = this->_getRow(categoryItem);
            }

            return this->index(begin, 0);

        }


};