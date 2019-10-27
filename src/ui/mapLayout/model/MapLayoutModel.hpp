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
                
                if(type == Payload::Alteration::Reset) {
                    
                    qDeleteAll(this->_atomsByAtomId);
                    this->_atomsByAtomId.clear();    

                    this->_atomsByAssetHash.clear();  

                    for(auto &category : this->_categories) {
                        qDeleteAll(category);
                    }
                    this->_categories.clear();

                    this->beginResetModel();

                }

                for (auto &atom : mPayload->atoms()) {
                    this->_createAtom(atom);
                }

                if(type == Payload::Alteration::Reset) this->endResetModel();
                else this->endInsertRows();

            }

            //on remove
            else if(auto mPayload = dynamic_cast<const RemovedPayload*>(payload)) {

                for (auto &id : mPayload->targetRPZAtomIds()) {
                    
                    //remove from internal list
                    auto atom = this->_atomsByAtomId.take(id);
                    auto atomId = atom->atomId();

                    if(RPZAtom::isLayoutAtom(atom->type())) {
                        this->_atomsByType[atom->type()].remove(atomId);
                    } 
                    
                    else {
                        this->_atomsByLayer[atom->layer()].remove(atomId);
                    }

                    //if has assetHash, remove it from tracking list
                    auto hash = atom->assetHash();
                    if(!hash.isNull()) {
                        this->_atomsByAssetHash[hash].remove(atomId);
                    }

                }
            }

            //on metadata change
            else if(auto mPayload = dynamic_cast<const MetadataChangedPayload*>(payload)) {
                
                auto updates = mPayload->updates();
                
                for (auto &id : mPayload->targetRPZAtomIds()) {
                    auto atom = this->_atomsByAtomId.value(id);
                    auto updated = atom->updateFrom(updates);
                    emit dataChanged(index, index, {Qt::DisplayRole});
                    //TODO
                }

            }


            //on metadata change (bulk)
            else if(auto mPayload = dynamic_cast<const BulkMetadataChangedPayload*>(payload)) {
                
                QHash<QTreeWidgetatom*, AtomUpdates> toUpdate;
                auto updatesById = mPayload->atomsUpdates();
                
                for (auto i = updatesById.begin(); i != updatesById.end(); i++) {
                    
                    auto atom = this->_atomsByAtomId.value(i.key());
                    auto updates = i.value();

                    toUpdate.insert(atom, updates);
                    this->_handleatomMove(atom, updates, mvHelper);
                }

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