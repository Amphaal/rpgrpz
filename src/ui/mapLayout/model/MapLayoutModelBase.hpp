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
                if(index.column()) continue;
                auto id = fromIndex(index);
                if(id) selectedIds += id;
            }

            return selectedIds;

        }

        static const RPZAtomId fromIndex(const QModelIndex &index) {
            
            auto item = MapLayoutItem::fromIndex(index);

            auto atom = dynamic_cast<MapLayoutAtom*>(item);
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
                return this->countCategories();
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
        QHash<RPZAtomId, MapLayoutAtom*> _atomsByAtomId;
        QHash<RPZAssetHash, QSet<RPZAtomId>> _atomsByAssetHash;
        QMap<RPZAtom::Category, QMap<int, MapLayoutCategory*>> _categories;

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

        MapLayoutCategory* _removeAtom(const RPZAtomId &toRemove) {
            
            //prepare
            auto atom = this->_atomsByAtomId.value(toRemove);
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
            
            for(auto &categoryItems : this->_categories) {
                
                auto count = categoryItems.count();
                
                if(row >= count) {
                    row = row - count;
                    continue;
                }

                auto sorter = categoryItems.keys().at(row);
                return categoryItems.value(sorter);

            }

            return nullptr;

        }

        int countCategories() const {
            auto total = 0;
            for(auto &categoryItems : this->_categories) {
                total += categoryItems.count();
            }
            return total;
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

        QModelIndex _getCategoryIndex(const RPZAtomLayer &layer, bool* created) {
            return this->_mayCreateCategory(RPZAtom::Category::Layout, layer, created);
        }

        QModelIndex _getCategoryIndex(MapLayoutCategory* category) {
            return this->index(this->_getRow(category), 0);
        }

    private:
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
            }

            return out;

        }

        QModelIndex _mayCreateCategory(const RPZAtom::Category &category, const int &sorter, bool* created = nullptr) {
            
            auto begin = -1;
            auto categoryItem = this->_categories[category].value(sorter);
                
            if(!categoryItem) {
                categoryItem = new MapLayoutCategory(category, sorter);
                this->_categories[category].insert(sorter, categoryItem);

                begin = this->_getRow(categoryItem);
                this->beginInsertRows(QModelIndex(), begin, begin);
                this->endInsertRows();

                if(created) *created = true;

            } 
            
            else {
                begin = this->_getRow(categoryItem);
            }

            return this->index(begin, 0);

        }


};