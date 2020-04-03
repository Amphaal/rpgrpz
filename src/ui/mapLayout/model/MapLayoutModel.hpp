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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "MapLayoutModelBase.hpp"
#include "src/shared/payloads/Payloads.h"
#include "src/shared/async-ui/AlterationHandler.h"
#include "src/shared/database/AssetsDatabase.h"

class MapLayoutModel : public MapLayoutModelBase {

    Q_OBJECT

    public slots:
        void propagateFocus(const QModelIndex &focusedIndex) {
            
            auto id = fromIndex(focusedIndex);
            if(!id) return;

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

                    this->beginResetModel();
                        this->_clearAll();
                    this->endResetModel();

                    this->beginResetModel();

                        for (auto &atom : mPayload->atoms()) {
                            this->_createAtom(atom, false);
                        }

                    this->endResetModel();

                } 
                
                //INSERT
                else if (type == Payload::Alteration::Added) {
                    
                    //inserts
                    for (auto &atom : mPayload->atoms()) {
                        this->_createAtom(atom);
                    }         

                }

            }

            //on remove
            else if(auto mPayload = dynamic_cast<const RemovedPayload*>(payload)) {
                
                QSet<MapLayoutCategory*> alteredCategories;

                //remove atoms
                for (auto &id : mPayload->targetRPZAtomIds()) {
                    auto alteredCategory = this->_removeAtom(id);
                    alteredCategories.insert(alteredCategory);
                }

                //clean empty categories
                for(const auto category : alteredCategories) {
                    this->_maybeRemoveCategory(category);
                }
                
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
        using LayerMoves = QHash<RPZAtom::Layer, QPair<QSet<MapLayoutAtom*>, QSet<MapLayoutAtom*>>>;
        
        struct LayerMoveContext {
            bool isCategoryCreated = false;
            QModelIndex categoryIndex;
            MapLayoutCategory* category = nullptr;
            QSet<MapLayoutAtom*> in;
            QSet<MapLayoutAtom*> out;
        };

        QSet<RPZAtom::Id> _bufferedSelectedAtomsIds;

        void _handleLayerMoves(const LayerMoves &moves) {
            
            //no moves, stop
            if(!moves.count()) return;

                QList<LayerMoveContext> contexts;

                //generate contexts
                for(auto i = moves.begin(); i != moves.end(); i++) {
                    
                    LayerMoveContext context;

                    context.categoryIndex = this->_getCategoryIndex(i.key(), &context.isCategoryCreated); //create non existing layers
                    context.category = MapLayoutCategory::fromIndex(context.categoryIndex);
                    context.in = i.value().first;
                    context.out = i.value().second;
                
                    //add to contexts
                    contexts += context;

                }

                //deletes
                for(const auto &context : contexts) {
                    
                    //skip if no out
                    if(!context.out.count()) continue;
                    
                    auto end = context.category->atomsCount() - 1;

                    this->beginRemoveRows(context.categoryIndex, 0, end);
                        for(const auto outAtom : context.out) {
                            outAtom->setParent(nullptr);
                        }
                    this->endRemoveRows(); 

                }

                //inserts + tree clearing + updates
                for(const auto &context : contexts) {

                    //inserts if necessary
                    auto inCount = context.in.count();
                    if(inCount) {
                        
                        //prepare insert ranges
                        auto begin = context.category->atomsCount();
                        auto end = begin + inCount;
                        
                        this->beginInsertRows(context.categoryIndex, begin, end);
                            for(const auto inAtom : context.in) {
                                inAtom->setParent(context.category);
                            }
                        this->endInsertRows();

                    }

                    //check if remove is necessary, if so, delete it and skip the rest
                    if(this->_maybeRemoveCategory(context.category)) continue;

                    // check if sold has evolved
                    auto inOutSold = inCount - context.out.count();
                    
                    //if so, refresh atom count on that category
                    if(inOutSold != 0 && !context.isCategoryCreated) {
                        auto categoryCountIndex = context.categoryIndex.siblingAtRow(2);
                        emit dataChanged(categoryCountIndex, categoryCountIndex, {Qt::DisplayRole});
                    }

                }

        }
        
        bool _maybeRemoveCategory(MapLayoutCategory* maybeToRemove) {
                
            //has atoms, skip
            if(maybeToRemove->atomsCount()) return false;
            
            //no more atoms, delete category
            this->_removeCategory(maybeToRemove);
            return true;

        }

        void _handleUpdates(const RPZAtom::Id &id, const RPZAtom::Updates &updates, LayerMoves &moves) {
            
            auto atom = this->_atomsByAtomId.value(id);

            //update category
            if(updates.contains(RPZAtom::Parameter::Layer)) {
                
                auto newLayer = updates.value(RPZAtom::Parameter::Layer).toInt();
                auto oldLayer = atom->parent()->sorter();
                
                moves[newLayer].first += atom;  //in   
                moves[oldLayer].second += atom; //out

            };

            //update atom
            auto colsToUpdate = atom->updateFrom(updates);
            if(!colsToUpdate.count()) return;

            auto atomIndex = this->toIndex(id);
            
            //signal redraw
            for(auto i = colsToUpdate.begin(); i != colsToUpdate.end(); i++) {
                auto simbling = atomIndex.siblingAtColumn(i.key());
                emit dataChanged(simbling, simbling, i.value().values().toVector());
            }

        }
    
    private slots:
        void _onRenamedAsset(const RPZAsset::Hash &id, const QString &newName) {

            for(const auto &id : this->_atomsByAssetHash.value(id)) {

                auto index = this->toIndex(id);
                auto atom = MapLayoutAtom::fromIndex(index);
                auto acked = atom->notifyAssetNameChange(newName);
                if(acked) emit dataChanged(index, index, {Qt::DisplayRole});

            }

        }
};