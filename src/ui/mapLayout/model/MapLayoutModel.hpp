#pragma once

#include "MapLayoutModelBase.hpp"
#include "src/shared/payloads/Payloads.h"
#include "src/shared/async-ui/AlterationHandler.h"
#include "src/shared/database/AssetsDatabase.h"

typedef QHash<RPZAtomLayer, QPair<int, int>> LayerMoves;

class MapLayoutModel : public MapLayoutModelBase {

    Q_OBJECT

    public slots:
        void propagateFocus(const QModelIndex &focusedIndex) {
            auto id = fromIndex(focusedIndex);
            FocusedPayload payload(id);
            AlterationHandler::get()->queueAlteration(Payload::Source::Local_MapLayout, payload);

        }

        void propagateSelection(const QModelIndexList &selectedIndexes) {
            auto selectedIds = fromIndexes(selectedIndexes);
            SelectedPayload payload(selectedIds);
            AlterationHandler::get()->queueAlteration(Payload::Source::Local_MapLayout, payload);
        }

    public:
        MapLayoutModel() {
            
            //on rename
            QObject::connect(
                AssetsDatabase::get(), &AssetsDatabase::assetRenamed,
                this, &MapLayoutModel::_onRenamedAsset
            );

        }

        void handleAlterationRequest(const AlterationPayload* payload) {
            
            auto type = payload->type();

            //atom wielders format (eg INSERT / RESET)
            if(auto mPayload = dynamic_cast<const AtomsWielderPayload*>(payload)) {
                
                //RESET
                if(type == Payload::Alteration::Reset) {
                    
                    this->_clearAll();

                    this->beginResetModel();

                        for (auto &atom : mPayload->atoms()) {
                            this->_createAtom(atom);
                        }

                    this->endResetModel();

                } 
                
                //INSERT
                else if (type == Payload::Alteration::Added) {
                    
                    QHash<QModelIndex, int> inserts;

                    //inserts
                    for (auto &atom : mPayload->atoms()) {
                        auto index = this->_createAtom(atom, true);
                        inserts[index.parent()]++;
                    }

                    //redraws
                    for(auto i = inserts.begin(); i != inserts.end(); i++) {
                        auto category = MapLayoutCategory::fromIndex(i.key());
                        
                        auto begin = category->atomsCount() - i.value();
                        auto end = category->atomsCount() - 1;
                        
                        this->beginInsertRows(i.key(), begin, end);
                    }

                    this->endInsertRows();

                }

            }

            //on remove
            else if(auto mPayload = dynamic_cast<const RemovedPayload*>(payload)) {
                
                QHash<MapLayoutCategory*, int> countRemovesByCategory;
                
                //remove atoms
                for (auto &id : mPayload->targetRPZAtomIds()) {
                    
                    auto alteredCategory = this->_removeAtom(id);
                    countRemovesByCategory[alteredCategory]++;

                }

                //define redraws
                auto mustRemoveFromRoot = false;
                for(auto i = countRemovesByCategory.begin(); i != countRemovesByCategory.end(); i++) {
                    
                    auto remainingAtoms = i.key()->atomsCount();
                    
                    //no atoms, delete category
                    if(!remainingAtoms) {
                        this->_removeCategory(i.key());
                        mustRemoveFromRoot = true;
                        continue;
                    }

                    //get category index
                    auto categoryRow = this->_getRow(i.key());
                    auto categoryIndex = this->index(categoryRow, 0, QModelIndex());

                    //update atom count
                    auto categoryIndexAtomCountIndex = categoryIndex.siblingAtColumn(2);
                    emit dataChanged(categoryIndexAtomCountIndex, categoryIndexAtomCountIndex, {Qt::DisplayRole});

                    //redraw
                    auto end = remainingAtoms - 1 + i.value();
                    this->beginRemoveRows(categoryIndex, 0, end);
                    
                }

                if(mustRemoveFromRoot) {
                    this->beginRemoveRows(QModelIndex(), 0, this->countCategories() - 1);
                }

                this->endRemoveRows();

            }

            //on metadata change
            else if(auto mPayload = dynamic_cast<const MetadataChangedPayload*>(payload)) {
                
                LayerMoves categoryMoves;

                auto updates = mPayload->updates();
                for (auto &id : mPayload->targetRPZAtomIds()) {
                    this->_handleUpdates(id, updates, categoryMoves);
                }

                this->_handleLayerMoves(categoryMoves);

            }


            //on metadata change (bulk)
            else if(auto mPayload = dynamic_cast<const BulkMetadataChangedPayload*>(payload)) {
                
                LayerMoves categoryMoves;

                auto updatesById = mPayload->atomsUpdates();
                for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
                    this->_handleUpdates(i.key(), i.value(), categoryMoves);
                }

                this->_handleLayerMoves(categoryMoves);

            }
            
        }
        
    private:
        void _handleLayerMoves(const LayerMoves &moves) {
            
            //no moves, stop
            if(!moves.count()) return;

            this->beginResetModel();

                //create non existing layers
                for(auto i = moves.begin(); i != moves.end(); i++) {
                    auto category = this->_mayCreateCategory(i.key());
                }

            this->endResetModel();


        }

        void _handleUpdates(const RPZAtomId &id, const AtomUpdates &updates, LayerMoves &moves) {
            
            auto atom = this->_atomsByAtomId.value(id);
            
            //update category
            if(updates.contains(AtomParameter::Layer)) {
                
                auto newLayer = updates.value(AtomParameter::Layer).toInt();
                auto oldLayer = atom->parent()->sorter();
                
                moves[oldLayer].second++; //out
                moves[newLayer].first++;  //in                 

            };

            //update atom
            auto colsToUpdate = atom->updateFrom(updates);
            if(!colsToUpdate.count()) return;

            auto atomIndex = this->toIndex(id);
            
            //signal redraw
            for(auto &col : colsToUpdate) {
                auto simbling = atomIndex.siblingAtColumn(col);
                emit dataChanged(simbling, simbling, {Qt::DisplayRole});
            }

        }
    
    private slots:
        void _onRenamedAsset(const RPZAssetHash &id, const QString &newName) {

            for(auto &id : this->_atomsByAssetHash.value(id)) {

                auto index = this->toIndex(id);
                auto atom = MapLayoutAtom::fromIndex(index);
                atom->setName(newName);
                emit dataChanged(index, index, {Qt::DisplayRole});

            }

        }
};