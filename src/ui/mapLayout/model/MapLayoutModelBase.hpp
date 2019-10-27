#pragma once

#include <QAbstractItemModel>
#include <QHash>

#include "MapLayoutAtom.hpp"
#include "MapLayoutCategory.hpp"

class MapLayoutModelBase : public QAbstractItemModel {
    public:
        MapLayoutModelBase() {}

        static const QVector<RPZAtomId> fromIndexes(const QModelIndexList &selectedIndexes) {
            QVector<RPZAtomId> selectedIds;
            for(auto &index : selectedIndexes) {
                selectedIds += fromIndex(index);
            }
            return selectedIds;
        }

        static const RPZAtomId fromIndex(const QModelIndex &index) {
            auto atom = MapLayoutAtom::fromIndex(index);
            return atom->atomId();
        }

        const QModelIndex toIndex(const RPZAtomId &id) {
            //TODO
            auto atom = this->_atomsByAtomId.value(id);
            auto category = atom->parent();

            auto categoryRow = this->_getRow(category);
            auto atomRow = category->rowOfAtom(atom);

            auto categoryIndex = this->index(categoryRow, 0);
            auto atomIndex = 
        }

        const QModelIndexList toIndexes(const QVector<RPZAtomId> &ids) {
            QModelIndexList out;
            for(auto &id : ids) {
                out += this->toIndex(id);
            }
            return out;
        }

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override {
            
            //if parent is not valid (eg Root), requires a category
            if(!parent.isValid()) {
                return this->createIndex(row, column, this->_getCategory(row));
            }

            auto base = MapLayoutItem::fromIndex(parent);
            
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
                return category->atoms().count();
            }

        }

        int columnCount(const QModelIndex &parent = QModelIndex()) const override {
             return 3;
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
            
            if(!index.isValid()) return QVariant();
            
            auto data = MapLayoutItem::fromIndex(index);

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
                                return QString::number(category->atoms().count());
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

            //category
            if(auto category = dynamic_cast<MapLayoutCategory*>(base)) {
                return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled);
            } 
            
            //atom
            else if(auto atom = dynamic_cast<MapLayoutAtom*>(base)) {
                return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable);
            }

        }

    protected:
        QHash<RPZAtomId, MapLayoutAtom*> _atomsByAtomId;
        QHash<RPZAssetHash, QSet<RPZAtomId>> _atomsByAssetHash;
        QMap<RPZAtom::Category, QMap<int, MapLayoutCategory*>> _categories;

        MapLayoutAtom* _createAtom(const RPZAtom &atom) {
            
            auto category = this->_mayCreateCategory(atom);
            auto mAtom = new MapLayoutAtom(category, atom);
            auto atomId = mAtom->atomId();

            this->_atomsByAtomId.insert(atomId, mAtom);
            
            //if has assetHash, add it
            auto assetHash = mAtom->assetHash();
            if(!assetHash.isNull()) {
                this->_atomsByAssetHash[assetHash].insert(atomId);
            }
            
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

    private:
        

        MapLayoutCategory* _mayCreateCategory(const RPZAtom &atom) {
            
            MapLayoutCategory * catatom = nullptr;
            auto category = atom.category();

            switch(category) {
                
                case RPZAtom::Category::Layout: {
                    auto layer = atom.layer();
                    catatom = this->_categories[category].value(layer);
                    if(!catatom) {
                        catatom = new MapLayoutCategory(layer);
                        this->_categories[category].insert(layer, catatom);
                    }
                }
                break;
                
                case RPZAtom::Category::Interactive: {
                    auto type = atom.type();
                    catatom = this->_categories[category].value((int)type);
                    if(!catatom) {
                        catatom = new MapLayoutCategory(type);
                        this->_categories[category].insert((int)type, catatom);
                    }
                }
                break;

                default:
                break;

            }

            return catatom;

        }



};