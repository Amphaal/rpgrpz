#pragma once

#include <QAbstractItemModel>
#include <QHash>

#include "src/shared/models/RPZAtom.h"

#include "MapLayoutAtom.h"
#include "MapLayoutCategory.h"

class MapLayoutModelBase : public QAbstractItemModel {
    public:
        MapLayoutModelBase() {}

        static const QVector<RPZAtomId> fromIndexes(const QModelIndexList &selectedIndexes) {
            
            QVector<RPZAtomId> selectedIds;
            
            for(auto &index : selectedIndexes) {
                auto id = fromIndex(index);
                if(!id) selectedIds += id;
            }

            return selectedIds;

        }

        static const RPZAtomId fromIndex(const QModelIndex &index) {
            
            auto atom = MapLayoutAtom::fromIndex(index);
            if(!atom) return 0;
            
            return atom->atomId();

        }

        const QModelIndex toIndex(const RPZAtomId &id) {
            
            auto atom = this->_atomsByAtomId.value(id);
            auto category = atom->parent();

            auto categoryRow = this->_getRow(category);
            auto atomRow = category->rowOfAtom(atom);

            auto categoryIndex = this->index(categoryRow, 0);
            auto atomIndex = this->index(atomRow, 0, categoryIndex);

            return atomIndex;
        }

        const QModelIndexList toIndexes(const QVector<RPZAtomId> &ids) {
            QModelIndexList out;
            for(auto &id : ids) {
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
                return this->createIndex(row, column, this->_getCategory(row));
            }

            auto base = MapLayoutItem::fromIndex(parent);
            if(!base) return QModelIndex();
            
            //category
            if(auto category = dynamic_cast<MapLayoutCategory*>(base)) {
                return this->createIndex(row, column, category->atom(row));
            } 

            return QModelIndex();
    
        }

        QModelIndex parent(const QModelIndex &child) const override {
            
            //root, have no parent
            if(!child.isValid()) return QModelIndex();

            auto base = MapLayoutItem::fromIndex(child);
            if(!base) return QModelIndex();
            
            //category, root is parent
            if(auto category = dynamic_cast<MapLayoutCategory*>(base)) {
                return QModelIndex();
            } 
            
            //atom, category is parent
            else if(auto atom = dynamic_cast<MapLayoutAtom*>(base)) {
                auto category = atom->parent();
                return this->createIndex(this->_getRow(category), 0, category);
            }

            return QModelIndex();

        }
    
        int rowCount(const QModelIndex &parent = QModelIndex()) const override {
            
            //from root, count categories to display
            if(!parent.isValid()) {
                auto count = 0;
                for(auto &set : this->_categories) count += set.count();
                return count;
            }

            //from category, count atoms
            else {
                
                auto category = MapLayoutCategory::fromIndex(parent);
                if(!category) return 0;

                return category->atomsCount();

            }

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
                    return index.column() == 1 ? Qt::AlignRight : QVariant();
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
        QHash<RPZAtomId, MapLayoutAtom*> _atomsByAtomId;
        QHash<RPZAssetHash, QSet<RPZAtomId>> _atomsByAssetHash;
        QMap<RPZAtom::Category, QMap<int, MapLayoutCategory*>> _categories;

        QModelIndex _createAtom(const RPZAtom &atom, bool fetchIndex = false) {
            
            auto category = this->_mayCreateCategory(atom);
            auto mAtom = new MapLayoutAtom(category, atom);
            auto atomId = mAtom->atomId();

            this->_atomsByAtomId.insert(atomId, mAtom);
            
            //if has assetHash, add it
            auto assetHash = mAtom->assetHash();
            if(!assetHash.isNull()) {
                this->_atomsByAssetHash[assetHash].insert(atomId);
            }
            
            return fetchIndex ? this->toIndex(atomId) : QModelIndex();

        }

        MapLayoutCategory* _removeAtom(const RPZAtomId &toRemove) {
            auto atom = this->_atomsByAtomId.take(toRemove);
            auto atomId = atom->atomId();

            //if has assetHash, remove it from tracking list
            auto hash = atom->assetHash();
            if(!hash.isNull()) {
                this->_atomsByAssetHash[hash].remove(atomId);
            }

            auto category = atom->parent();
            delete atom;

            return category;

        }

        MapLayoutCategory* _getCategory(int row) const {
            
            for(auto &categoryItems : this->_categories) {
                
                auto count = categoryItems.count();
                
                if(row >= count) {
                    row = row - count;
                    continue;
                }

                return categoryItems.value(row);

            }

            return nullptr;

        }

        int countCategories() {
            auto total = 0;
            for(auto &categoryItems : this->_categories) {
                total += categoryItems.count();
            }
            return total;
        }

        int _getRow(MapLayoutCategory* category) const {
            auto row = -1;
            
            for(auto &categoryItems : this->_categories) {
                for(auto &item : categoryItems) {
                    row++;
                    if(item == category) return row;
                }
            }

            return -1;
        }

        void _clearAll() {

            qDeleteAll(this->_atomsByAtomId);
            this->_atomsByAtomId.clear();    

            this->_atomsByAssetHash.clear();  

            for(auto &category : this->_categories) {
                qDeleteAll(category);
            }
            this->_categories.clear();

        }

        void _removeCategory(MapLayoutCategory* toRemove) {
            this->_categories[toRemove->category()].remove(toRemove->sorter());
            delete toRemove;
        }


        MapLayoutCategory* _mayCreateCategory(const RPZAtomLayer &layer) {
            return this->_mayCreateCategory(RPZAtom::Category::Layout, layer);
        }

    private:
        MapLayoutCategory* _mayCreateCategory(const RPZAtom &atom) {
            
            MapLayoutCategory * categoryItem = nullptr;
            
            auto category = atom.category();
            switch(category) {
                case RPZAtom::Category::Layout: {
                    categoryItem = this->_mayCreateCategory(category, atom.layer());
                }
                break;

                case RPZAtom::Category::Interactive: {
                    categoryItem = this->_mayCreateCategory(category, (int)atom.type());
                }
                break;
            }

            return categoryItem;

        }

        MapLayoutCategory* _mayCreateCategory(const RPZAtom::Category &category, const int &sorter) {
            auto categoryItem = this->_categories[category].value(sorter);
                
            if(!categoryItem) {
                categoryItem = new MapLayoutCategory(category, sorter);
                this->_categories[category].insert(sorter, categoryItem);
            }

            return categoryItem;
        }


};